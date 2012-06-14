#!/usr/bin/python3

import sys, os, atexit, signal
import socketserver
import queue
import threading
import serial
import traceback
import time

STATUS_DIR    = "/tmp"  
PID_FILE      = STATUS_DIR + "/serialhandler.pid"

global queue

# @atexit.register
def unlink():
  print("unlinking", PID_FILE)
  os.unlink(PID_FILE)

def findpid():
  f = open(PID_FILE)
  p = f.read()
  f.close()
  return int(p)

def createpid():
  atexit.register (unlink)
  f = open(PID_FILE, "w")
  f.write("%d" % os.getpid())
  f.close()

# wird nicht benutzt, ignore
class EnhancedSerial(serial.Serial):
    def __init__(self, *args, **kwargs):
        #ensure that a reasonable timeout is set
        timeout = kwargs.get('timeout',0.1)
        if timeout < 0.01: timeout = 0.1
        kwargs['timeout'] = timeout
        serial.Serial.__init__(self, *args, **kwargs)
        self.buf = ''
        
    def readline(self, maxsize=None, timeout=1):
        """maxsize is ignored, timeout in seconds is the max time that is way for a complete line"""
        tries = 0
        while 1:
            self.buf += self.read(512)
            print ("buf: %s" % self.buf)
            pos = self.buf.find('\r\n')
            if pos >= 0:
                line, self.buf = self.buf[:pos+2], self.buf[pos+2:]
                return line
            tries += 1
            if tries * self.timeout > timeout:
                break
        line, self.buf = self.buf, ''
        return line

class Watcher:
    """this class solves two problems with multithreaded
    programs in Python, (1) a signal might be delivered
    to any thread (which is just a malfeature) and (2) if
    the thread that gets the signal is waiting, the signal
    is ignored (which is a bug).

    The watcher is a concurrent process (not thread) that
    waits for a signal and the process that contains the
    threads.  See Appendix A of The Little Book of Semaphores.
    http://greenteapress.com/semaphores/
    """
    
    def __init__(self):
        """ Creates a child thread, which returns.  The parent
            thread waits for a KeyboardInterrupt and then kills
            the child thread.
        """
        self.child = os.fork()
        if self.child == 0:
            return
        else:
            self.watch()

    def watch(self):
        try:
            os.wait()
        except KeyboardInterrupt:
            print('KeyboardInterrupt')
            self.kill()
        sys.exit()

    def kill(self):
        try:
            os.kill(self.child, signal.SIGKILL)
        except OSError: pass

class MyTCPHandler(socketserver.BaseRequestHandler):

  def handle (self):
    global queue
    print("created handle")
    while True:
      line = queue.get ()
      # print ("got line: ", line)
      if len (line) > 0:
        self.request.send(line)
        print ("sent line: %s" % line)
      queue.task_done()

class MyTCPServer (socketserver.TCPServer):

  def __init__(self, address, handler):
    self.allow_reuse_address = True
    socketserver.TCPServer.__init__(self, address, handler)

#  def handle_error(self, request, client_address):
#    print("socket error: ", request.__class__.__name__, client_address)
        
class ServerThread (threading.Thread):
    
  def run (self):
    # Create the server, binding to localhost on port 4711
    self.server = MyTCPServer(("localhost", 4711), MyTCPHandler)
    print("server created: ", self.server.server_address)
      
    while True:
      self.server.handle_request ()

class SerialThread (threading.Thread):
  def __init__ (self):
    threading.Thread.__init__(self)
    self.serial = None
    
  def createSerial (self):
    success = False
    # we try the USB device first.
    if not success:
      try:
        self.serial = serial.Serial('/dev/ttyUSB0', 19200, timeout=1)
        success = True
      except serial.serialutil.SerialException as exc:
        print ("no ttyUSB0 device: ", exc)
      else:
        print("serial created: ttyUSB0")

    if not success:
      try:
        self.serial = serial.Serial('/dev/ttyUSB1', 19200, timeout=1)
        success = True
      except serial.serialutil.SerialException as exc:
        print("no ttyUSB1 device: ", exc)
        time.sleep (5)
        self.serial = None
      else:
        print("serial created: ttyUSB1")

    # this is the normal case. No USB device, we connect to the Bluetooth device
    if not success:
      try:
        self.serial = serial.Serial('/dev/rfcomm0', 19200, timeout=1)
        success = True;
      except serial.serialutil.SerialException as exc:
        print ("no bluetooth device: ", exc)
      else:
        print("serial created: rfcomm0")      
    
  def run (self):
    global queue

    while True:
      # if the device has been destroyed, recreate. This is forced by setting it to None
      if self.serial == None:
        self.createSerial ()
        continue
        
      try:
        #line = self.serial.readline (None, '\r\n')
        line = self.serial.readline ()
        if len (line) == 0:
          # print ("empty line: %s" % line)
          # this happens after timeout; the call to getRI causes an exception if the device has been disconnected
          # we use this as a side effect to force to recreate the device after timeout
          ri = self.serial.getRI ()
          ## should the call work and the line is down, recreate
          #if not ri:
          #   print ("RI is down, try to reconnect")
          #   time.sleep (5)
          #   self.serial = None
          continue

      except serial.serialutil.SerialException as exc:
        # this happens when the cable is disconnected, but not on the Stylistic
        print("SerialException: ", exc)
        self.serial.close()
        time.sleep(1)
        self.serial = None
        continue

      except serial.portNotOpenError as exc:
        print("portNotOpenError serial exception: ", exc)
        self.serial.close()
        time.sleep(1)
        self.serial = None
        continue

      except IOError as exc:
        print("IOError serial exception: ", exc)
        self.serial.close()
        time.sleep(5)
        self.serial = None
        continue

      except Exception as exc:
        print("other serial exception: ", exc, exc.__class__.__name__)
        self.serial.close()
        time.sleep(5)
        self.serial = None
        continue
          
      # print ("put: %s" % line)
      print ("type of line: ", line.__class__.__name__)
      if queue.full ():
        print("throw away: ", queue.get())
      else:
        queue.put(line)

class FileThread (threading.Thread):
  def __init__ (self, filename):
    threading.Thread.__init__(self)
    self.serial = None
    self.filename = filename
    
  def run (self):
    global queue

    file = open (self.filename, 'r')
    while True:
      time.sleep (0.02)
      # the socket does not accept str which is unicode
      line = bytes (file.readline (), 'ascii')

      if len (line) == 0:
        continue

      # print ("put: %s" % line)
      if queue.full ():
        print("throw away: ", queue.get())
      else:
        queue.put(line)

if __name__ == "__main__":
  from optparse import OptionParser

  parser = OptionParser()
  parser.add_option("-f", "--file", dest="filename", help="read data from FILE", metavar="FILE")

  (options, args) = parser.parse_args()
  if options.filename:
    print ("Read from %s" % options.filename)
  else:
    print ("Read from serial")

  if os.path.exists(PID_FILE):
    pid = findpid()
    # pid file exists, check for real process
    if os.path.isdir ("/proc/" + str (pid)):
      # real serialhandler is running
      print("Process with PID = %d is running. Exiting" % pid)
      sys.exit (1)
    else:
      # the file was a relict. delete it and continue
      print("Stale pidfile exists with PID = %d; Removing it." % pid)
      os.unlink(PID_FILE)
                                                                                                    
  #create new pid file
  createpid()

  Watcher ()

  global queue
  queue = queue.Queue(10)

  if options.filename:
    serialThread = FileThread (options.filename)
  else:
    serialThread = SerialThread ()
  
  serverThread = ServerThread ()

  serialThread.start ()
  serverThread.start ()

  serialThread.join()
  serverThread.join()

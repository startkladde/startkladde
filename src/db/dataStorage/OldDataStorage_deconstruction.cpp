bool DataStorage::sleep (OperationMonitor &monitor, int seconds)
{
	std::cout << "DataStorage waiting for " << seconds << " seconds on " << (isGuiThread ()?"the GUI thread":"a background thread") << std::endl;

	int i;
	for (i=0; i<seconds && !monitor.isCanceled (); ++i)
	{
		monitor.progress (i, seconds);
		DefaultQThread::msleep (1000);
	}

	return (i==seconds);
}

Vagrant.configure(2) do |config|
  config.vm.box = "trusty64"

  config.vm.box_url = "http://files.vagrantup.com/trusty64.box"

  config.vm.network :forwarded_port, host: 3306, guest: 3306

  config.vm.provision "shell", inline: <<-SHELL
    apt-get update
    export DEBIAN_FRONTEND=noninteractive
    debconf-set-selections <<< 'mysql-server-5.1 mysql-server/root_password password '
    debconf-set-selections <<< 'mysql-server-5.1 mysql-server/root_password_again password '
    apt-get install -y mysql-client mysql-server
    mysql -u root -e 'create database if not exists startkladde;'
    mysql -u root -e 'grant all on startkladde.* to startkladde@"%" identified by "sk";'
    sed -i 's/skip-external-locking//' /etc/mysql/my.cnf
    sed -i 's/.*bind-address.*/bind-address = 0.0.0.0/' /etc/mysql/my.cnf
    service mysql restart
  SHELL
end

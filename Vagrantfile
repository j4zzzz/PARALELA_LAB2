Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/focal64"

  # Configuración global reducida para máquinas con pocos recursos
  config.vm.provider "virtualbox" do |vb|
    vb.memory = 500   # 500 MB de RAM
    vb.cpus = 1       # 1 CPU
  end

  $install_mpi = <<-SHELL
    sudo apt-get update -y
    sudo apt-get install -y build-essential openmpi-bin openmpi-common libopenmpi-dev
  SHELL

  $master_setup = <<-SHELL
    # Generar clave si no existe
    if [ ! -f /home/vagrant/.ssh/id_rsa ]; then
      ssh-keygen -t rsa -b 2048 -f /home/vagrant/.ssh/id_rsa -q -N ""
      chown vagrant:vagrant /home/vagrant/.ssh/id_rsa*
    fi
    # Guardar clave pública en carpeta compartida
    cp /home/vagrant/.ssh/id_rsa.pub /vagrant/master_key.pub
  SHELL

  # Script para workers: agregar clave del master
  $worker_setup = <<-SHELL
    if [ -f /vagrant/master_key.pub ]; then
      mkdir -p /home/vagrant/.ssh
      cat /vagrant/master_key.pub >> /home/vagrant/.ssh/authorized_keys
      chown -R vagrant:vagrant /home/vagrant/.ssh
      chmod 600 /home/vagrant/.ssh/authorized_keys
    fi
  SHELL

  # MASTER
  config.vm.define "master" do |master|
    master.vm.hostname = "master"
    master.vm.network "private_network", ip: "192.168.20.10"
    master.vm.provision "shell", inline: $install_mpi
    master.vm.provision "shell", inline: $master_setup, run: "always"
  end

  # CPU 1
  config.vm.define "cpu1" do |cpu|
    cpu.vm.hostname = "cpu1"
    cpu.vm.network "private_network", ip: "192.168.20.11"
    cpu.vm.provision "shell", inline: $install_mpi
    cpu.vm.provision "shell", inline: $worker_setup, run: "always"
  end

  # CPU 2
  config.vm.define "cpu2" do |cpu|
    cpu.vm.hostname = "cpu2"
    cpu.vm.network "private_network", ip: "192.168.20.12"
    cpu.vm.provision "shell", inline: $install_mpi
    cpu.vm.provision "shell", inline: $worker_setup, run: "always"
  end

  # CPU 3
  config.vm.define "cpu3" do |cpu|
    cpu.vm.hostname = "cpu3"
    cpu.vm.network "private_network", ip: "192.168.20.13"
    cpu.vm.provision "shell", inline: $install_mpi
    cpu.vm.provision "shell", inline: $worker_setup, run: "always"
  end
end

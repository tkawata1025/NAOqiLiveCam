
cd NAOqi

#qibuild configure nao_interface
#qibuild install nao_interface ../../build
qibuild configure --release nao_interface
qibuild install --release nao_interface ../../build

cd ..

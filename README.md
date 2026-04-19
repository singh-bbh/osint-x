## Setup for Users

1. Clone the repo
git clone https://github.com/singh-bbh/osint-x

2. Install dependencies
sudo apt-get install -y libcurl4-openssl-dev cmake build-essential libboost-all-dev libasio-dev

3. Install Crow
git clone https://github.com/CrowCpp/Crow.git include/crow

4. Build : 
If there is already a build folder in osint-x/osint-x run the command : rm -rf build |
mkdir build && cd build |
cmake .. |
make -j4 

5. Run CLI
./osint-x ip 8.8.8.8

6. Run Web UI
./osint-web
open http://localhost:8080
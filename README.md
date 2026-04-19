## Setup for teammates

1. Clone the repo
git clone https://github.com/YOUR_USERNAME/osint-x

2. Install dependencies
sudo apt-get install -y libcurl4-openssl-dev cmake build-essential libboost-all-dev libasio-dev

3. Install Crow
git clone https://github.com/CrowCpp/Crow.git include/crow

4. Build
mkdir build && cd build
cmake ..
make -j4

5. Run CLI
./osint-x ip 8.8.8.8

6. Run Web UI
./osint-web
open http://localhost:8080
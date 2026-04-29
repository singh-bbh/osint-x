## Setup for Users

1. Clone the repo
git clone https://github.com/singh-bbh/osint-x

2. Install dependencies
sudo apt-get install -y libcurl4-openssl-dev cmake build-essential libboost-all-dev libasio-dev

3. Install Crow
git clone https://github.com/CrowCpp/Crow.git include/crow

4. Go to CMakeLists.txt and edit the include_directories to (
                                                    src
                                                    include
                                                    {Change the Workspace to the directory in which your osint-x is saved root, downloads etc.})

5. Build : 
If there is already a build folder in osint-x/osint-x run the command : rm -rf build 
mkdir build && cd build 
cmake .. 
make -j4 

6. Run CLI
./osint-x ip 8.8.8.8

7. Run Web UI
./osint-web
open http://localhost:8080
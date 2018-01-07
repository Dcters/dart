brew update > /dev/null

PACKAGES='
git
cmake
assimp
dartsim/dart/fcl
bullet --with-double-precision
ode --with-libccd --with-double-precision
flann
boost
eigen
tinyxml
tinyxml2
dartsim/dart/libccd
nlopt
dartsim/dart/ipopt
ros/deps/urdfdom
ros/deps/urdfdom_headers
ros/deps/console_bridge
open-scene-graph
'

brew install $PACKAGES | grep -v '%$'

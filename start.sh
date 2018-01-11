#!/bin/bash

pushd $(dirname "$0")

nohup bin/fcserver fcserver-config.json &> /var/log/fcserver.log &
nohup ./poll-camera.js | bin/LightpathSim/LightpathSim &> /var/log/LightpathSim.log &

popd

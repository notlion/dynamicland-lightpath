#!/bin/bash

pushd $(dirname "$0")

nohup bin/fcserver fcserver-config.json &> /var/log/fcserver.log &
nohup bin/LightpathSim/LightpathSim &> /var/log/LightpathSim.log &

popd

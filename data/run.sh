#!/bin/sh

CURR_DIR=$(dirname $0)
./bin/cellular_automaton $(cat data/config/local)

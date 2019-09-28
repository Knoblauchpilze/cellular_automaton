#!/bin/sh

CURR_DIR=$(dirname $0)

gdb --args ./bin/cellular_automaton $(cat data/config/local)

#!/bin/bash

ps -ef | grep 'runs' | grep -v grep | awk '{print $2}' | xargs -r kill -9

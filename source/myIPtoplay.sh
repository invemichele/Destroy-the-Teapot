#!/bin/bash
#small script that returns your local IP address, using 'ifconfig'. works well on Ubuntu and on @tolab, not tested elsewhere.
echo $( ifconfig  | grep 'inet:' |grep -v '127.0.0.1'| cut -f2 -d:| awk '{print $1}'&& ifconfig  | grep 'inet addr:' |grep -v '127.0.0.1'| cut -f2 -d:| awk '{print $1}')

#!/bin/sh

sed 's#^.*[ \t]\([^ \t]*\);.*$#\tOUT(\1);#g'

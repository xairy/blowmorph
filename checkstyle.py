#!/usr/bin/python

import sys
import subprocess

def check_file(filename):
  success = True
  try:
    subprocess.check_call(['python', 'cpplint.py', filename])
  except subprocess.CalledProcessError as error:
    success = False
  return success

def main():
  ok = True
  try:
    whitelist = open('whitelist', 'r')
  except:
    print 'Whitelist not found!'
    sys.exit(False)
  for filename in whitelist:
    ok = ok and check_file(filename.rstrip())
  whitelist.close()
  sys.exit(not(ok))

if __name__ == '__main__':
  main()

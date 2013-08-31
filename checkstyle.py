#!/usr/bin/python

import sys
import subprocess

def hg_get_commit_changes():
  files = str(subprocess.check_output(['hg', 'status', '-man']))
  return [line for line in files.splitlines() if line != '']

def hg_get_all_files():
  files = str(subprocess.check_output(['hg', 'status', '-man', '--all']))
  return [line for line in files.splitlines() if line != '']

def get_src_files():
  files = hg_get_all_files()
  extensions = ['c', 'cpp', 'h', 'hpp']
  return [file for file in files if file.split('/')[0] == 'src' and file.split('.')[-1] in extensions]

def get_blacklisted_files():
  with open('blacklist', 'r') as blacklist:
    return [line.rstrip() for line in blacklist if line != '' and line.lstrip()[0] != '#']

def cpplint_check_file(filename):
  try:
    subprocess.check_call(['python', 'cpplint.py', filename])
  except subprocess.CalledProcessError as error:
    return False
  return True

def main():
  src_files = get_src_files()
  blacklisted_files = get_blacklisted_files()
  files_to_check = [file for file in src_files if file not in blacklisted_files]

  bad_files_count = 0
  for file in files_to_check:
    if not cpplint_check_file(file):
      bad_files_count += 1

  sys.exit(bad_files_count)

if __name__ == '__main__':
  main()

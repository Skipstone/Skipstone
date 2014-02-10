from waflib import Logs
import subprocess, sys

top = '.'
out = 'build'

def options(ctx):
	ctx.load('pebble_sdk')

def configure(ctx):
	ctx.load('pebble_sdk')

def build(ctx):
	ctx.load('pebble_sdk')

	ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'), target='pebble-app.elf')

	cli('jshint %s/appinfo.json' % (ctx.path.abspath()))
	cli('jshint %s/js/*.js' % (ctx.path.abspath()))
	cli('uglifyjs %s/js/*.js -o src/js/pebble-js-app.js -cm' % (ctx.path.abspath()))

	ctx.pbl_bundle(elf='pebble-app.elf', js='src/js/pebble-js-app.js')

def cli(cmd):
	Logs.pprint('YELLOW', cmd)
	ret = subprocess.call(cmd, shell=True)
	if not ret == 0:
		sys.exit(ret)

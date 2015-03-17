#!/usr/bin/python

from __future__ import unicode_literals

import json
import sys

def id_to_tile(id, tiled_map):
  last_tileset = None
  for tileset in tiled_map['tilesets']:
    if id >= tileset['firstgid']:
      last_tileset = tileset
      continue
  tileset_name = last_tileset['image'].split('/')[-1].split('.')[0]
  tile_id = id - last_tileset['firstgid']
  return (tileset_name, tile_id)

def tile_to_sprite(tileset_name, tile_id):
  m = {
    'land': ['land_grass', 'land_concrete', 'land_brick', 'land_wood'],
    'hitech': [None, None, None, 'hitech_plate']
  }
  return m[tileset_name][tile_id]

def get_terrain_layer(tiled_map):
  for layer in tiled_map['layers']:
    if layer['name'] == 'Terrain':
      return layer
  assert False

assert len(sys.argv) == 3
src = sys.argv[1]
dst = sys.argv[2]

tiled_map = json.loads(open(src).read())

terrain = []
for id in get_terrain_layer(tiled_map)['data']:
  terrain.append(tile_to_sprite(*id_to_tile(id, tiled_map)))

terrain = str(terrain)
terrain = terrain.replace("u'", '"')
terrain = terrain.replace("'", '"')
print terrain

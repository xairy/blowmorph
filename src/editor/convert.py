#!/usr/bin/python

from __future__ import unicode_literals

import json
import sys

def id_to_tile(id, tm):
  assert id > 0
  last_tileset = None
  for tileset in tm['tilesets']:
    if id >= tileset['firstgid']:
      last_tileset = tileset
      continue
  tileset_name = last_tileset['image'].split('/')[-1].split('.')[0]
  tile_id = id - last_tileset['firstgid']
  return (tileset_name, tile_id)

def terrain_tile_to_sprite(tileset_name, tile_id):
  m = {
    'land': ['land_grass', 'land_concrete', 'land_brick', 'land_wood'],
    'hitech': [None, None, None, 'hitech_plate']
  }
  return m[tileset_name][tile_id]

def entity_tile_to_entity(tileset_name, tile_id):
  m = {
    'walls': [None, 'morphed_wall', 'unbreakable_wall', 'ordinary_wall']
  }
  return m[tileset_name][tile_id]

def get_layer(tm, layer_name):
  for layer in tm['layers']:
    if layer['name'] == layer_name:
      return layer
  assert False

assert len(sys.argv) == 2
src = sys.argv[1]

tm = json.loads(open(src).read())

m = {}
m['width'] = tm['width']
m['height'] = tm['height']
m['block_size'] = float(tm['tilewidth'])

m['terrain'] = []
for id in get_layer(tm, 'Terrain')['data']:
  m['terrain'].append(terrain_tile_to_sprite(*id_to_tile(id, tm)))

m['walls'] = []
for i, id in enumerate(get_layer(tm, 'Entity')['data']):
  if id == 0:
    continue
  x = i % m['width']
  y = i / m['width']
  entity = entity_tile_to_entity(*id_to_tile(id, tm))
  wall = {'entity': entity, 'x': x, 'y': y}
  m['walls'].append(wall)

m['spawns'] = [{'x': 10, 'y': 10}]
m['kits'] = [{'entity': 'health_kit', 'x': 1, 'y': 1}]

print json.dumps(m, indent=4, separators=(',', ': '))

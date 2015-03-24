#!/usr/bin/python

from __future__ import unicode_literals

import json
import sys

terrain_tilesets = {
  'land': ['land_grass', 'land_concrete', 'land_brick', 'land_wood'],
  'hitech': [None, None, None, 'hitech_plate']
}

door_tilesets = {
  'door': ['door']
}

wall_tilesets = {
  'walls': [None, 'morphed_wall', 'unbreakable_wall', 'ordinary_wall']
}

def merge_dicts(*dict_args):
  result = {}
  for dictionary in dict_args:
    result.update(dictionary)
  return result

def terrain_tile_to_sprite_name(tileset_name, tile_id):
  return terrain_tilesets[tileset_name][tile_id]

def entity_tile_to_entity_name(tileset_name, tile_id):
  entity_tilesets = merge_dicts(door_tilesets, wall_tilesets)
  return entity_tilesets[tileset_name][tile_id]

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
  m['terrain'].append(terrain_tile_to_sprite_name(*id_to_tile(id, tm)))

m['spawns'] = [{'x': 10, 'y': 10}]

m['kits'] = [{'entity': 'health_kit', 'x': 1, 'y': 1}]

m['doors'] = []
m['walls'] = []
for i, id in enumerate(get_layer(tm, 'Entity')['data']):
  if id == 0:
    continue
  x = i % m['width']
  y = i / m['width']
  tileset_name, tile_id = id_to_tile(id, tm)
  entity_name = entity_tile_to_entity_name(tileset_name, tile_id)
  entity = {'entity': entity_name, 'x': x, 'y': y}
  if tileset_name in door_tilesets.keys():
    m['doors'].append(entity)
  elif tileset_name in wall_tilesets.keys():
    m['walls'].append(entity)
  else:
    assert False

print json.dumps(m, indent=4, separators=(',', ': '))

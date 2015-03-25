#-*- coding: utf-8 -*-
# Copyright (c) 2015 Blowmorph Team

from __future__ import unicode_literals
import ast, json, requests, subprocess, sys, wx
from launcher_frame_base import LauncherFrameBase

##############################################################################
class LauncherFrame(LauncherFrameBase):
    #-------------------------------------------------------------------------
    def __init__(self):
        LauncherFrameBase.__init__(self, None)
        self.selected_address = None
        self.SetupLayout()

    #-------------------------------------------------------------------------
    def SetupLayout(self):
        self.LoadConfig()

        # Bind buttons.
        self.Bind(wx.EVT_BUTTON, self.OnQuitButtonPressed,
            self.quit_button)
        self.Bind(wx.EVT_BUTTON, self.OnSaveButtonPressed,
            self.save_button)
        self.Bind(wx.EVT_BUTTON, self.OnConnectButtonPressed,
            self.connect_button)
        self.Bind(wx.EVT_BUTTON, self.OnRefreshButtonPressed,
            self.refresh_button)

        column_names = ["Name", "Address"]
        for i in xrange(len(column_names)):
            self.server_list.InsertColumn(i, column_names[i])

        self.RefreshGameServerList()

        # Bind list events.
        self.Bind(wx.EVT_LIST_ITEM_SELECTED,
            self.OnItemSelected, self.server_list)
        self.Bind(wx.EVT_LIST_ITEM_DESELECTED,
            self.OnItemDeselected, self.server_list)
        self.Bind(wx.EVT_LIST_ITEM_ACTIVATED,
            self.OnItemActivated, self.server_list)

        self.nickname_text.SetValue(self.cfg_name)

        # Set resolution choice.
        item_count = self.resolution_choice.GetCount()
        for i in xrange(item_count):
            item = self.resolution_choice.GetString(i)
            if item.find(str(self.cfg_width)) != -1 and \
               item.find(str(self.cfg_height)) != -1:
                self.resolution_choice.SetSelection(i)

        # Set fullscreen checkbox.
        self.fullscreen_checkbox.SetValue(self.cfg_fullscreen)

        # Set advanced settings.
        self.master_server_text.SetValue(
            "%s:%d" % (self.cfg_ms_host, self.cfg_ms_port))
        self.connection_timeout_spin.SetValue(self.cfg_connection_timeout)
        self.sync_timeout_spin.SetValue(self.cfg_sync_timeout)
        self.max_misposition_spin.SetValue(self.cfg_max_misposition)
        self.interpolation_offset_spin.SetValue(self.cfg_interpolation_offset)
        self.tick_rate_spin.SetValue(self.cfg_tick_rate)

        self.Layout()

    #-------------------------------------------------------------------------
    def OnQuitButtonPressed(self, event):
        self.Destroy()

    #-------------------------------------------------------------------------
    def OnSaveButtonPressed(self, event):
        self.SaveConfig()

    #-------------------------------------------------------------------------
    def OnConnectButtonPressed(self, event):
        if self.selected_address == None:
            return
        self.SaveConfig()
        subprocess.call("./client.sh")

    #-------------------------------------------------------------------------
    def OnRefreshButtonPressed(self, event):
        ms_addr = self.master_server_text.GetValue().split(':')
        self.cfg_ms_host, self.cfg_ms_port = ms_addr[0], int(ms_addr[1])
        self.RefreshGameServerList()

    #-------------------------------------------------------------------------
    def OnItemSelected(self, event):
        item = self.server_list.GetItem(event.GetIndex(), 1)
        self.selected_address = item.GetText()

    #-------------------------------------------------------------------------
    def OnItemDeselected(self, event):
        self.selected_address = None

    #-------------------------------------------------------------------------
    def OnItemActivated(self, event):
        item = self.server_list.GetItem(event.GetIndex(), 1)
        self.selected_address = item.GetText()
        self.OnConnectButtonPressed(event = None)

    #-----------------------------------------------------------------------------
    def LoadConfig(self):
        config_file = open('data/client.json')
        config = json.loads(config_file.read())
        config_file.close()

        self.cfg_width = config["graphics"]["width"]
        self.cfg_height = config["graphics"]["height"]
        self.cfg_fullscreen = config["graphics"]["fullscreen"]

        self.cfg_name = config["player"]["name"]

        self.cfg_ms_host = config["master-server"]["host"]
        self.cfg_ms_port = config["master-server"]["port"]

        self.cfg_connection_timeout = config["net"]["connect_timeout"]
        self.cfg_sync_timeout =  config["net"]["sync_timeout"]
        self.cfg_max_misposition = config["net"]["max_player_misposition"]
        self.cfg_interpolation_offset = config["net"]["interpolation_offset"]
        self.cfg_tick_rate = config["net"]["tick_rate"]

    #-----------------------------------------------------------------------------
    def RefreshGameServerList(self):
        servers = {}
        try:
            address = "http://%s:%d" % (self.cfg_ms_host, self.cfg_ms_port)
            r = requests.get(address)
            servers = ast.literal_eval(r.text)
        except requests.exceptions.ConnectionError:
            print "Couldn't connect to master server: %s:%d!" % \
              (self.cfg_ms_host, self.cfg_ms_port)
            return

        self.server_list.DeleteAllItems()

        for key, value in servers.items():
            name = value["name"]
            addr = "%s:%d" % (value["host"], value["port"])
            index = self.server_list.InsertStringItem(sys.maxint, name)
            self.server_list.SetStringItem(index, 1, addr)

        for i in xrange(self.server_list.GetColumnCount()):
            self.server_list.SetColumnWidth(i, self.server_list.GetSize()[0] / 2)

    #-------------------------------------------------------------------------
    def SaveConfig(self):
        config = {
            "server": {},
            "master-server": {},
            "graphics": {},
            "player": {},
            "net": {}
        }

        if self.selected_address != None:
            address = self.selected_address.split(":")
            host, port = address[0], int(address[1])

            config["server"]["host"] = host
            config["server"]["port"] = port

        selection = self.resolution_choice.GetSelection()
        resolution = self.resolution_choice.GetString(selection)
        resolution = resolution.split('(')[0].split('x')
        width, height = int(resolution[0]), int(resolution[1])
        fullscreen = self.fullscreen_checkbox.GetValue()

        config["graphics"]["width"] = width
        config["graphics"]["height"] = height
        config["graphics"]["fullscreen"] = fullscreen

        nickname = self.nickname_text.GetValue()

        config["player"]["name"] = nickname

        ms_addr = self.master_server_text.GetValue().split(':')
        ms_host, ms_port = ms_addr[0], int(ms_addr[1])

        config["master-server"]["host"] = ms_host
        config["master-server"]["port"] = ms_port

        connection_timeout = self.connection_timeout_spin.GetValue()
        sync_timeout = self.sync_timeout_spin.GetValue()
        max_misp = float(self.max_misposition_spin.GetValue())
        inter_offset = self.interpolation_offset_spin.GetValue()
        tick_rate = self.tick_rate_spin.GetValue()

        config["net"]["connect_timeout"] = connection_timeout
        config["net"]["sync_timeout"] = sync_timeout
        config["net"]["max_player_misposition"] = max_misp
        config["net"]["interpolation_offset"] = inter_offset
        config["net"]["tick_rate"] = tick_rate

        config_file = open('data/client.json', 'w')
        config_file.write(json.dumps(config, indent=4, separators=(',', ': ')))
        config_file.close()
    #-------------------------------------------------------------------------
##############################################################################

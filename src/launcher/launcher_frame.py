#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

from __future__ import unicode_literals
import ast, requests, subprocess, sys, wx
from pylibconfig import Config
from launcher_frame_base import LauncherFrameBase

def load_config(config, name):
    value = config.value(name.encode("utf8"))
    assert value[1] == True
    value = value[0]
    if type(value) == str:
        value = value.decode("utf8")
    return value

def save_config(config, name, value):
    if type(value) == unicode:
        value = value.encode("utf8")
    config.setValue(name.encode("utf8"), value)

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

        self.nickname_text.SetValue(self.cfg_login)

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
        self.config = Config()
        self.config.readFile("data/client.cfg".encode("utf8"))

        self.cfg_width = load_config(self.config, "graphics.width")
        self.cfg_height = load_config(self.config, "graphics.height")
        self.cfg_fullscreen = load_config(self.config, "graphics.fullscreen")

        self.cfg_login = load_config(self.config, "player.login")

        self.cfg_ms_host = load_config(self.config, "master-server.host")
        self.cfg_ms_port = load_config(self.config, "master-server.port")

        self.cfg_connection_timeout = \
            load_config(self.config, "client.connect_timeout")
        self.cfg_sync_timeout = \
            load_config(self.config, "client.sync_timeout")
        self.cfg_max_misposition = \
            load_config(self.config, "client.max_player_misposition")
        self.cfg_interpolation_offset = \
            load_config(self.config, "client.interpolation_offset")
        self.cfg_tick_rate = \
            load_config(self.config, "client.tick_rate")

    #-----------------------------------------------------------------------------
    def RefreshGameServerList(self):
        servers = {}
        try:
            address = "http://%s:%d" % (self.cfg_ms_host, self.cfg_ms_port)
            r = requests.get(address)
            servers = ast.literal_eval(r.text)
        except requests.exceptions.ConnectionError:
            print "Could connect to master server: %s:%d!" % \
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
        if self.selected_address != None:
            address = self.selected_address.split(":")
            host, port = address[0], int(address[1])

            save_config(self.config, "server.host", host)
            save_config(self.config, "server.port", port)

        selection = self.resolution_choice.GetSelection() 
        resolution = self.resolution_choice.GetString(selection)
        resolution = resolution.split('(')[0].split('x')
        width, height = int(resolution[0]), int(resolution[1])
        fullscreen = self.fullscreen_checkbox.GetValue()

        save_config(self.config, "graphics.width", width)
        save_config(self.config, "graphics.height", height)
        save_config(self.config, "graphics.fullscreen", fullscreen)

        nickname = self.nickname_text.GetValue()

        save_config(self.config, "player.login", nickname)

        ms_addr = self.master_server_text.GetValue().split(':')
        ms_host, ms_port = ms_addr[0], int(ms_addr[1])

        save_config(self.config, "master-server.host", ms_host)
        save_config(self.config, "master-server.port", ms_port)

        connection_timeout = self.connection_timeout_spin.GetValue()
        sync_timeout = self.sync_timeout_spin.GetValue()
        max_misp = float(self.max_misposition_spin.GetValue())
        inter_offset = self.interpolation_offset_spin.GetValue()
        tick_rate = self.tick_rate_spin.GetValue()

        save_config(self.config, "client.connect_timeout", connection_timeout)
        save_config(self.config, "client.sync_timeout", sync_timeout)
        save_config(self.config, "client.max_player_misposition", max_misp)
        save_config(self.config, "client.interpolation_offset", inter_offset)
        save_config(self.config, "client.tick_rate", tick_rate)

        self.config.writeFile("data/client.cfg".encode("utf8"))
    #-------------------------------------------------------------------------
##############################################################################

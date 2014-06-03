#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

from __future__ import unicode_literals
from pylibconfig import Config
import ast, requests, subprocess, sys
import wx
from launcher_frame_base import LauncherFrameBase

def load_config(config, name):
    value = config.value(name.encode("utf8"))
    assert value[1] == True
    return value[0]

##############################################################################
class LauncherFrame(LauncherFrameBase):
    #-------------------------------------------------------------------------
    def __init__(self, title="Blowmorph Launcher", size=(330, 400)):
        LauncherFrameBase.__init__(self, None)
        self.selected_address = None
        self.LoadConfig()
        self.RetrieveGameServerList()
        self.SetupLayout()

    #-------------------------------------------------------------------------
    def SetupLayout(self):
        self.Bind(wx.EVT_BUTTON, self.OnConnectButtonPressed,
            self.connect_button)
        self.Bind(wx.EVT_BUTTON, self.OnQuitButtonPressed,
            self.quit_button)

        column_names = ["Name", "Address"]
        for i in xrange(len(column_names)):
            self.server_list.InsertColumn(i, column_names[i])
            
        server_list_data = [[self.servers_dict[key][0], key] 
                            for key in self.servers_dict]
        
        # FIXME(xairy): wtf.
        for item in server_list_data:
            index = self.server_list.InsertStringItem(sys.maxint, str(item[0]))
            for p in xrange(len(item) - 1):
                self.server_list.SetStringItem(index, p + 1, str(item[p + 1]))
        
        for i in xrange(len(column_names)):
            self.server_list.SetColumnWidth(i, self.server_list.GetSize()[0] / 2)

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

        self.Layout()

    #-------------------------------------------------------------------------
    def OnQuitButtonPressed(self, event):
        self.Destroy()

    #-------------------------------------------------------------------------
    def OnConnectButtonPressed(self, event):
        if self.selected_address == None:
            return

        address = self.selected_address.split(":")
        host, port = address[0], int(address[1])

        selection = self.resolution_choice.GetSelection() 
        resolution = self.resolution_choice.GetString(selection)
        resolution = resolution.split('(')[0].split('x')
        width, height = int(resolution[0]), int(resolution[1])

        fullscreen = self.fullscreen_checkbox.GetValue()
        nickname = self.nickname_text.GetValue()

        self.SaveConfig(host=host, port=port, width=width,
            height=height, fullscreen=fullscreen, login=nickname)

        subprocess.call("./client.sh")

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

    #-----------------------------------------------------------------------------
    def RetrieveGameServerList(self):
        try:
            address = "http://%s:%d" % (self.cfg_ms_host, self.cfg_ms_port)
            r = requests.get(address)
            self.servers_dict = ast.literal_eval(r.text)
        except requests.exceptions.ConnectionError:
            print "Could connect to master server: %s:%d!" % \
              (self.cfg_ms_host, self.cfg_ms_port)
            self.servers_dict = {}

    #-------------------------------------------------------------------------
    def SaveConfig(self, host, port, width, height, fullscreen, login):
        self.config.setValue("server.host".encode("utf8"), host.encode("utf8"))
        self.config.setValue("server.port".encode("utf8"), port)
        self.config.setValue("graphics.width".encode("utf8"), width);
        self.config.setValue("graphics.height".encode("utf8"), height);
        self.config.setValue("graphics.fullscreen".encode("utf8"), fullscreen);
        self.config.setValue("player.login".encode("utf8"), login.encode("utf8"));
        self.config.writeFile("data/client.cfg".encode("utf8"))
    #-------------------------------------------------------------------------
##############################################################################

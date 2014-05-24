#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

from __future__ import unicode_literals

import ast
import requests
import subprocess
import sys
import wx

##############################################################################
class MainFrame(wx.Frame):
    """First (and main) frame class."""

    #-------------------------------------------------------------------------
    def __init__(self, title = "Blowmorph launcher", size = (330, 400)):
        wx.Frame.__init__(self, None, title = title, size = size)
        self.list_ctrl = None
        self.selected_address = None
        self.load_cfg()
        self.DoLayout()

    #-------------------------------------------------------------------------
    # Current format of textctrls (tc):
    # | nick_label | nick(tc) | resolution_label | w(tc) | separator | h(tc) |
    def CreateTextCtrls(self):
        self.panel_nickname = wx.Panel(self)
        _nick_label  = wx.StaticText(self.panel_nickname, label="Nick: ",
                                    pos = wx.Point(5, 4))
        self.nickname = wx.TextCtrl(self.panel_nickname, size=(100, -1),
                                    pos = wx.Point(45, 0), 
                                    value = self.cfg_login)
        
        self.panel_resolution = wx.Panel(self)
        _resolution_label = wx.StaticText(self.panel_resolution, -1,
                                              label="Resolution: ",
                                              pos = wx.Point(5, 4))
        
        self.width_resolution = wx.TextCtrl(self.panel_resolution, 
                                            size=(45, -1),
                                            pos = wx.Point(77, 0),
                                            value = self.cfg_width)
        self.height_resolution = wx.TextCtrl(self.panel_resolution,
                                             pos = wx.Point(135, 0),
                                             size=(45, -1),
                                             value = self.cfg_height)

        _separator = wx.StaticText(self.panel_resolution, -1, 
                                  label="x",
                                  pos = wx.Point(125, 4))
    #-----------------------------------------------------------------------------
    def load_cfg(self):
        # Fix it with python-libconfig.
        file = open("../../data/client.cfg")
        lines = file.readlines()
        for line in lines:
            if len(line) >= 9 and line[0:9] == "  host = ":
                host = line[10:-3]
            if len(line) >= 9 and line[0:9] == "  port = ":
                port = line[9:-2]
            if len(line) >= 10 and line[0:10] == "  width = ":
                self.cfg_width = line[10:-2]
            if len(line) >= 11 and line[0:11] == "  height = ":
                self.cfg_height = line[11:-2]
            if len(line) >= 10 and line[0:10] == "  login = ":
                self.cfg_login = line[11:-3]
        
        host = "localhost"        
        address = "http://"+ host + ":" + port
        r = requests.get(address)
        self.servers_dict = ast.literal_eval(r.text)
                
    #-------------------------------------------------------------------------
    def CreateListCtrl(self):
        self.list_ctrl = wx.ListCtrl(self, wx.ID_ANY, style = wx.LC_REPORT)
        
        # Create columns.
        column_names = ["Name", "IP"]
        for i in xrange(len(column_names)):
            self.list_ctrl.InsertColumn(i, column_names[i])
            
        # Fill the table.
        #test_data = [["xairy's server",  "198.168.0.1"], 
        #             ["rdkl's server",   "198.168.0.2"],
        #             ["andreyknvl.com",  "198.168.0.3"]]
        
        servers_data = [[name, self.servers_dict[name]] 
                        for name in self.servers_dict]
        
        for item in servers_data:
            index = self.list_ctrl.InsertStringItem(sys.maxint, 
                                                    str(item[0]))
            for p in xrange(len(item) - 1):
                self.list_ctrl.SetStringItem(index, p + 1, str(item[p + 1]))
        
        # Resize columns width.
        for i in xrange(len(column_names)):
            self.list_ctrl.SetColumnWidth(i, wx.LIST_AUTOSIZE)
        
        # Bind functions.
        self.Bind(wx.EVT_LIST_ITEM_SELECTED,   self.OnItemSelected, 
                  self.list_ctrl)
        self.Bind(wx.EVT_LIST_ITEM_DESELECTED, self.OnItemDeselected, 
                  self.list_ctrl)
        self.Bind(wx.EVT_LIST_ITEM_ACTIVATED,  self.OnItemActivated, 
                  self.list_ctrl)

    #-------------------------------------------------------------------------
    def DoLayout(self):
        parent_sizer = wx.BoxSizer(wx.VERTICAL)
        text_sizer = wx.BoxSizer(wx.HORIZONTAL)
        button_sizer = wx.BoxSizer(wx.HORIZONTAL)
        
        self.CreateTextCtrls()
        text_sizer.Add(self.panel_nickname, proportion = 0, 
                       flag = wx.EXPAND)
        
        text_sizer.Add(self.panel_resolution, proportion = 0, 
                             flag = wx.EXPAND)
        
        list_label = "Available servers"
        staticbox = wx.StaticBox(self, wx.NewId(), label = list_label)
        list_sizer = wx.StaticBoxSizer(staticbox, wx.VERTICAL)
        
        self.CreateListCtrl()            
        list_sizer.Add(self.list_ctrl, proportion = 1, flag = wx.EXPAND)
        
        self.button_connect = wx.Button(self, wx.ID_ANY, "Connect")
        self.button_close = wx.Button(self, wx.ID_EXIT)

        # button_sizer = |(connect)|--------blank--------|(quit)|.
        
        button_sizer.Add(self.button_close, proportion = 0, flag = wx.ALL, 
                         border = 2)
        button_sizer.Add(wx.Size(10, 10), proportion = 1, flag = wx.ALL, 
                         border = 2)
        button_sizer.Add(self.button_connect, proportion = 0, flag = wx.ALL, 
                         border = 2)
        
        parent_sizer.Add(wx.Size(10, 10), proportion = 0, flag = wx.EXPAND)
        parent_sizer.Add(text_sizer, proportion = 0, flag = wx.EXPAND)
        parent_sizer.Add(wx.Size(10, 10), proportion = 0, flag = wx.EXPAND)
        parent_sizer.Add(list_sizer, proportion = 1, flag = wx.EXPAND)
        parent_sizer.Add(button_sizer, flag = wx.EXPAND)
        
        self.SetSizer(parent_sizer)
        
        self.Bind(wx.EVT_BUTTON, self.OnButtonConnectPressed, 
                  self.button_connect)
        self.Bind(wx.EVT_BUTTON, self.OnButtonClosePressed, self.button_close)
        
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)
        
        self.Layout()        
    
    #-------------------------------------------------------------------------
    def OnItemSelected(self, event):
        self.selected_address = self.list_ctrl.GetItem(event.GetIndex(), 1)\
                                    .GetText()
        
    #-------------------------------------------------------------------------
    def OnItemDeselected(self, event):
        #item = event.GetItem()
        pass
        
    #-------------------------------------------------------------------------
    def OnItemActivated(self, event):
        self.selected_address = self.list_ctrl.GetItem(event.GetIndex(), 1)\
                                    .GetText()
        self.OnButtonConnectPressed(event = None)
        
    #-------------------------------------------------------------------------
    def OnCloseWindow(self, event):
        print "OnCloseWindow"
        self.Destroy()

    #-------------------------------------------------------------------------
    def OnButtonClosePressed(self, event):
        print "OnButtonClosePressed"
        self.OnCloseWindow(wx.EVT_CLOSE)
        
    #-------------------------------------------------------------------------
    def OnButtonConnectPressed(self, event):
        if self.selected_address == None:
            return
        
        self.write_cfg(address = self.selected_address,
                       width   = self.width_resolution.GetValue(),
                       height  = self.height_resolution.GetValue(), 
                       login   = self.nickname.GetValue())
        bash_command = "../.././client.sh"
        process = subprocess.Popen(bash_command.split(), shell=True,
                                   stdout=subprocess.PIPE)
        
        # TODO: close when connection established.
        output = process.communicate()[0]
        print output
         
    #-------------------------------------------------------------------------
    def write_cfg(self, address, width, height, login):
        f = open("../../data/client.cfg", "r")
        lines = f.readlines()
        f.close()
        f = open("../../data/client.cfg", "w+")
        
        for line in lines:
            if len(line) >= 9 and line[0:9] == "  host = ":
                line = "  host = \"" + address + "\";\n"
            if len(line) >= 10 and line[0:10] == "  width = ":
                line = "  width = " + width + ";\n"
            if len(line) >= 11 and line[0:11] == "  height = ":
                line = "  height = " + height + ";\n"
            if len(line) >= 10 and line[0:10] == "  login = ":
                line = "  login = \"" + login + "\";\n"
            if len(line) > 1:
                print >>f, line[:-1]
            
        f.close()
        
    #-------------------------------------------------------------------------
##############################################################################

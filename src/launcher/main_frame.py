#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

from __future__ import unicode_literals

from pylibconfig import Config
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
                                            value = str(self.cfg_width))
        self.height_resolution = wx.TextCtrl(self.panel_resolution,
                                             pos = wx.Point(135, 0),
                                             size=(45, -1),
                                             value = str(self.cfg_height))

        _separator = wx.StaticText(self.panel_resolution, -1, 
                                  label="x",
                                  pos = wx.Point(125, 4))
    #-----------------------------------------------------------------------------
    def load_cfg(self):
        self.config = Config()
        self.config.readFile("data/client.cfg".encode("utf8"))

        width = self.config.value("resolution.width".encode("utf8"))
        height = self.config.value("resolution.height".encode("utf8"))
        assert (width[1] == True) and (height[1] == True)
        self.cfg_width, self.cfg_height = width[0], height[0]

        login = self.config.value("player.login".encode("utf8"))
        assert login[1] == True
        self.cfg_login = login[0]

        host = self.config.value("master-server.host".encode("utf8"))
        port = self.config.value("master-server.port".encode("utf8"))
        assert (host[1] == True) and (port[1] == True)
        host, port = host[0], port[0]

        address = "http://"+ host + ":" + str(port)

        try:
            r = requests.get(address)
            self.servers_dict = ast.literal_eval(r.text)
        except requests.exceptions.ConnectionError:
            print "Could not get info from master server: ", address
            self.servers_dict = {}
                
    #-------------------------------------------------------------------------
    def CreateListCtrl(self):
        self.list_ctrl = wx.ListCtrl(self, wx.ID_ANY, style = wx.LC_REPORT)
        
        # Create columns.
        column_names = ["Name", "Address"]
        for i in xrange(len(column_names)):
            self.list_ctrl.InsertColumn(i, column_names[i])
            
        servers_data = [[self.servers_dict[key][0], key] 
                        for key in self.servers_dict]
        
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
        
        self.write_cfg(host   = self.selected_address.split(":")[0],
                       port   = int(self.selected_address.split(":")[1]),
                       width  = int(self.width_resolution.GetValue()),
                       height = int(self.height_resolution.GetValue()), 
                       login  = self.nickname.GetValue())
        bash_command = "./client.sh"
        process = subprocess.Popen(bash_command.split(), shell=True,
                                   stdout=subprocess.PIPE)
        
        # TODO: close when connection established.
        output = process.communicate()[0]
        print output
         
    #-------------------------------------------------------------------------
    def write_cfg(self, host, port, width, height, login):
        self.config.setValue("server.host".encode("utf8"), host.encode("utf8"))
        self.config.setValue("server.port".encode("utf8"), port)
        self.config.setValue("resolution.width".encode("utf8"), width);
        self.config.setValue("resolution.height".encode("utf8"), height);
        self.config.setValue("player.login".encode("utf8"), login.encode("utf8"));
        self.config.writeFile("data/client.cfg".encode("utf8"))
    #-------------------------------------------------------------------------
##############################################################################

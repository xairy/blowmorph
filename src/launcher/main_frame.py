#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

from __future__ import unicode_literals

import sys
import wx


class MainFrame(wx.Frame):
    """First (and main) frame class."""

    #-------------------------------------------------------------------------
    def __init__(self, title = "Blowmorph launcher", size = (330, 400)):
        wx.Frame.__init__(self, None, title = title, size = size)
        self.list_ctrl = None
        self.DoLayout()


    #-------------------------------------------------------------------------
    def CreateTextCtrls(self):
        self.panel_nickname = wx.Panel(self)
        nick_label = wx.StaticText(self.panel_nickname, label="Nick: ",
                                   pos = wx.Point(5, 4))
        
        self.nickname = wx.TextCtrl(self.panel_nickname, size=(100, -1),
                                    pos = wx.Point(45, 0), 
                                    value = "xairy")
        
        self.panel_resolution = wx.Panel(self)
        self.resolution_label = wx.StaticText(self.panel_resolution, -1,
                                              label="Resolution: ",
                                              pos = wx.Point(5, 4))
        
        self.width_resolution = wx.TextCtrl(self.panel_resolution, 
                                              size=(45, -1),
                                              pos = wx.Point(77, 0))
        separator = wx.StaticText(self.panel_resolution, -1, 
                                  label="x",
                                  pos = wx.Point(125, 4))
        self.heigth_resolution = wx.TextCtrl(self.panel_resolution,
                                             pos = wx.Point(135, 0),
                                             size=(45, -1))
        
    #-------------------------------------------------------------------------
    def CreateListCtrl(self):
        self.list_ctrl = wx.ListCtrl(self, wx.ID_ANY, style = wx.LC_REPORT)
        
        # Create columns.
        column_names = ["Name", "IP"]
        for i in xrange(len(column_names)):
            self.list_ctrl.InsertColumn(i, column_names[i])
            
        # Fill the table.
        test_data = [["xairy's server",  "198.168.0.1"], 
                     ["rdkl's server",   "198.168.0.2"],
                     ["andreyknvl.com",  "198.168.0.3"]]
        for item in test_data:
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
        item = event.GetItem()
        print "Item selected:", item.GetText()

    #-------------------------------------------------------------------------
    def OnItemDeselected(self, event):
        item = event.GetItem()
        print "Item deselected:", item.GetText()

    #-------------------------------------------------------------------------
    def OnItemActivated(self, event):
        file_name = self.list_ctrl.GetItem(event.GetIndex(), 1).GetText()
        print "Item activated:", file_name
        
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
        print "OnButtonConnectPressed"
        
    #-------------------------------------------------------------------------
##############################################################################

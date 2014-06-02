#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

from __future__ import unicode_literals

import wx
from main_frame import MainFrame

##############################################################################
class Launcher(wx.App):
    #-------------------------------------------------------------------------
    def __init__(self):
        super(Launcher, self).__init__()

    #-------------------------------------------------------------------------
    def OnInit(self):
        self.frame = MainFrame()
        self.frame.Center()
        self.frame.Show()
        self.SetTopWindow(self.frame)
        return True        
    #-------------------------------------------------------------------------
##############################################################################

if __name__ == "__main__":
    Launcher().MainLoop()

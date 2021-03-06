#-*- coding: utf-8 -*-
# Copyright (c) 2015 Blowmorph Team

from __future__ import unicode_literals
import wx
from launcher_frame import LauncherFrame

##############################################################################
class Launcher(wx.App):
    #-------------------------------------------------------------------------
    def __init__(self):
        super(Launcher, self).__init__()

    #-------------------------------------------------------------------------
    def OnInit(self):
        self.frame = LauncherFrame()
        self.frame.Center()
        self.frame.Show()
        self.SetTopWindow(self.frame)
        return True
    #-------------------------------------------------------------------------
##############################################################################

if __name__ == "__main__":
    Launcher().MainLoop()

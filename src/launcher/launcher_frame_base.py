# -*- coding: utf-8 -*- 

###########################################################################
## Python code generated with wxFormBuilder (version Feb 26 2014)
## http://www.wxformbuilder.org/
##
## PLEASE DO "NOT" EDIT THIS FILE!
###########################################################################

import wx
import wx.xrc

###########################################################################
## Class LauncherFrameBase
###########################################################################

class LauncherFrameBase ( wx.Frame ):
	
	def __init__( self, parent ):
		wx.Frame.__init__ ( self, parent, id = wx.ID_ANY, title = u"Blowmorph Launcher", pos = wx.DefaultPosition, size = wx.Size( 500,500 ), style = wx.DEFAULT_FRAME_STYLE|wx.TAB_TRAVERSAL )
		
		self.SetSizeHintsSz( wx.DefaultSize, wx.DefaultSize )
		
		launcher_sizer = wx.BoxSizer( wx.VERTICAL )
		
		self.settings_notebook = wx.Notebook( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.general_panel = wx.Panel( self.settings_notebook, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		general_sizer = wx.BoxSizer( wx.VERTICAL )
		
		nickname_sizer = wx.BoxSizer( wx.HORIZONTAL )
		
		self.nickname_label = wx.StaticText( self.general_panel, wx.ID_ANY, u"Nickname:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.nickname_label.Wrap( -1 )
		nickname_sizer.Add( self.nickname_label, 0, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		self.nickname_text = wx.TextCtrl( self.general_panel, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		nickname_sizer.Add( self.nickname_text, 1, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		
		general_sizer.Add( nickname_sizer, 0, wx.EXPAND, 5 )
		
		server_sizer = wx.StaticBoxSizer( wx.StaticBox( self.general_panel, wx.ID_ANY, u"Available servers" ), wx.VERTICAL )
		
		self.server_list = wx.ListCtrl( self.general_panel, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.LC_REPORT )
		server_sizer.Add( self.server_list, 1, wx.EXPAND, 5 )
		
		
		general_sizer.Add( server_sizer, 1, wx.EXPAND, 5 )
		
		graphics_sizer = wx.StaticBoxSizer( wx.StaticBox( self.general_panel, wx.ID_ANY, u"Graphics" ), wx.HORIZONTAL )
		
		self.resolution_label = wx.StaticText( self.general_panel, wx.ID_ANY, u"Resolution:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.resolution_label.Wrap( -1 )
		graphics_sizer.Add( self.resolution_label, 0, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		resolution_choiceChoices = [ u"800 x 600 (4:3)", u"1024 x 768 (4:3)", u"1280 x 960 (4:3)", u"1600 x 1200 (4:3)", u"1280 x 720 (16:9)", u"1600 x 900 (16:9)", u"1920 x 1080 (16:9)", u"2560 x 1440 (16:9)" ]
		self.resolution_choice = wx.Choice( self.general_panel, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, resolution_choiceChoices, 0 )
		self.resolution_choice.SetSelection( 0 )
		graphics_sizer.Add( self.resolution_choice, 1, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		self.fullscreen_checkbox = wx.CheckBox( self.general_panel, wx.ID_ANY, u"Fullscreen", wx.DefaultPosition, wx.DefaultSize, 0 )
		graphics_sizer.Add( self.fullscreen_checkbox, 0, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		
		general_sizer.Add( graphics_sizer, 0, wx.EXPAND, 5 )
		
		
		self.general_panel.SetSizer( general_sizer )
		self.general_panel.Layout()
		general_sizer.Fit( self.general_panel )
		self.settings_notebook.AddPage( self.general_panel, u"General", True )
		self.advanced_panel = wx.Panel( self.settings_notebook, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		advanced_sizer = wx.BoxSizer( wx.VERTICAL )
		
		maxter_server_sizer = wx.BoxSizer( wx.HORIZONTAL )
		
		self.master_server_label = wx.StaticText( self.advanced_panel, wx.ID_ANY, u"Master server address:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.master_server_label.Wrap( -1 )
		maxter_server_sizer.Add( self.master_server_label, 0, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		self.master_server_text = wx.TextCtrl( self.advanced_panel, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		maxter_server_sizer.Add( self.master_server_text, 1, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		
		advanced_sizer.Add( maxter_server_sizer, 0, wx.EXPAND, 5 )
		
		self.m_staticline1 = wx.StaticLine( self.advanced_panel, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.LI_HORIZONTAL )
		advanced_sizer.Add( self.m_staticline1, 0, wx.EXPAND |wx.ALL, 5 )
		
		timeouts_sizer = wx.BoxSizer( wx.HORIZONTAL )
		
		self.connection_timeout_label = wx.StaticText( self.advanced_panel, wx.ID_ANY, u"Connection timeout:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.connection_timeout_label.Wrap( -1 )
		timeouts_sizer.Add( self.connection_timeout_label, 1, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		self.connection_timeout_spin = wx.SpinCtrl( self.advanced_panel, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 80,-1 ), wx.SP_ARROW_KEYS, 1, 5000, 0 )
		timeouts_sizer.Add( self.connection_timeout_spin, 0, wx.ALL, 5 )
		
		self.sync_timeout_label = wx.StaticText( self.advanced_panel, wx.ID_ANY, u"Sync timeout:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.sync_timeout_label.Wrap( -1 )
		timeouts_sizer.Add( self.sync_timeout_label, 1, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		self.sync_timeout_spin = wx.SpinCtrl( self.advanced_panel, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 80,-1 ), wx.SP_ARROW_KEYS, 1, 5000, 0 )
		timeouts_sizer.Add( self.sync_timeout_spin, 0, wx.ALL, 5 )
		
		
		advanced_sizer.Add( timeouts_sizer, 0, wx.EXPAND, 5 )
		
		position_sizer = wx.BoxSizer( wx.HORIZONTAL )
		
		self.max_misposition_label = wx.StaticText( self.advanced_panel, wx.ID_ANY, u"Max misposition:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.max_misposition_label.Wrap( -1 )
		position_sizer.Add( self.max_misposition_label, 1, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		self.max_misposition_spin = wx.SpinCtrl( self.advanced_panel, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 80,-1 ), wx.SP_ARROW_KEYS, 1, 5000, 0 )
		position_sizer.Add( self.max_misposition_spin, 0, wx.ALL, 5 )
		
		self.interpolation_offset_label = wx.StaticText( self.advanced_panel, wx.ID_ANY, u"Interpolation offset:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.interpolation_offset_label.Wrap( -1 )
		position_sizer.Add( self.interpolation_offset_label, 1, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		self.interpolation_offset_spin = wx.SpinCtrl( self.advanced_panel, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 80,-1 ), wx.SP_ARROW_KEYS, 1, 5000, 0 )
		position_sizer.Add( self.interpolation_offset_spin, 0, wx.ALL, 5 )
		
		
		advanced_sizer.Add( position_sizer, 0, wx.EXPAND, 5 )
		
		tickrate_sizer = wx.BoxSizer( wx.HORIZONTAL )
		
		bSizer13 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.tick_rate_label = wx.StaticText( self.advanced_panel, wx.ID_ANY, u"Tick rate:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.tick_rate_label.Wrap( -1 )
		bSizer13.Add( self.tick_rate_label, 1, wx.ALIGN_CENTER|wx.ALL, 5 )
		
		self.tick_rate_spin = wx.SpinCtrl( self.advanced_panel, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 80,-1 ), wx.SP_ARROW_KEYS, 1, 100, 0 )
		bSizer13.Add( self.tick_rate_spin, 0, wx.ALL, 5 )
		
		
		tickrate_sizer.Add( bSizer13, 1, wx.EXPAND, 5 )
		
		
		tickrate_sizer.AddSpacer( ( 0, 0), 1, wx.EXPAND, 5 )
		
		
		advanced_sizer.Add( tickrate_sizer, 0, wx.EXPAND, 5 )
		
		
		self.advanced_panel.SetSizer( advanced_sizer )
		self.advanced_panel.Layout()
		advanced_sizer.Fit( self.advanced_panel )
		self.settings_notebook.AddPage( self.advanced_panel, u"Advanced", False )
		
		launcher_sizer.Add( self.settings_notebook, 1, wx.EXPAND |wx.ALL, 5 )
		
		control_sizer = wx.BoxSizer( wx.HORIZONTAL )
		
		self.quit_button = wx.Button( self, wx.ID_ANY, u"Quit", wx.DefaultPosition, wx.DefaultSize, 0 )
		control_sizer.Add( self.quit_button, 0, wx.ALL, 5 )
		
		
		control_sizer.AddSpacer( ( 0, 0), 1, wx.EXPAND, 5 )
		
		self.refresh_button = wx.Button( self, wx.ID_ANY, u"Refresh", wx.DefaultPosition, wx.DefaultSize, 0 )
		control_sizer.Add( self.refresh_button, 0, wx.ALL, 5 )
		
		self.save_button = wx.Button( self, wx.ID_ANY, u"Save", wx.DefaultPosition, wx.DefaultSize, 0 )
		control_sizer.Add( self.save_button, 0, wx.ALL, 5 )
		
		self.connect_button = wx.Button( self, wx.ID_ANY, u"Connect", wx.DefaultPosition, wx.DefaultSize, 0 )
		control_sizer.Add( self.connect_button, 0, wx.ALL, 5 )
		
		
		launcher_sizer.Add( control_sizer, 0, wx.EXPAND, 5 )
		
		
		self.SetSizer( launcher_sizer )
		self.Layout()
		
		self.Centre( wx.BOTH )
	
	def __del__( self ):
		pass
	


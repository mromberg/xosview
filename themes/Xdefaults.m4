! Xdefaults.  Generated from Xdefaults.in by configure.
! ######################################################################
! ######################################################################
! ##  DO NOT EDIT Xdefaults!!!!
! ##
! ##  Only make changes to the file Xdefaults.in, and re-run ./configure
! ##  to regenerate Xdefaults.
! ##
! ##  Thanks,
! ##    the Management :)    (bgrayson)
! ##
! ######################################################################
! ######################################################################
!
!  xosview resources (override into your .Xdefaults file)
!
! ######################################################################
! A note on comments:
! X only technically supports a comment by placing a '!' character
! and the beginning of a line.  Anything else appears to be undefined
! behavior.  Since xosview has used comments on any position on a line
! for 20+ years (and I like it this way) it will continue to do so.
!
! So, you may place a '!' anywhere *AFTER* the ':' and the rest of the
! line will be a comment.  Unless you wish to use a literal '!' character
! in your value.  Then "escape" the '!' with a '&' character like so:
! xosview*someName: True ! This is a comment
! xosview*someName: False&!  ! This resource is set to "False!"
! ######################################################################

! **General Resources
!xosview*title:
!xosview*geometry:           +5+5
!xosview*display:
xosview*captions:           True
xosview*labels:             True
xosview*usedlabels:         True
xosview*transparent:        False
xosview*pixmapName:         XPMFILE  ! tile and xpm as background
xosview*meterLabelColor:    LABEL1
xosview*usedLabelColor:     LABEL2
!xosview*font:               7x13bold      ! X11 core font
!xosview*font:               Oxygen Mono-8  ! Freetype2 font
xosview*font:               FONT
xosview*background:         BG1
xosview*foreground:         OUTLINE
xosview*enableStipple:	    False ! Change to true to try beta stipple support.
xosview*samplesPerSec:      10    ! not yet supported
xosview*graphNumCols:       128   ! number of samples shown in a graph


! ** Load Meter Resources
!    We do not need to enable the Decay option, as it is already time-averaged.
xosview*load:               True
xosview*loadCritColor:      CRIT
xosview*loadWarnColor:      WARN
xosview*loadProcColor:      FIELD1
xosview*loadIdleColor:      FIELD0
xosview*loadPriority:       20
xosview*loadWarnThreshold:  4     !  4 core system pretty common
xosview*loadCritThreshold:  6
xosview*loadDecay:          False
xosview*loadGraph:          False
xosview*loadUsedFormat:     float
xosview*loadCpuSpeed:       True


! ** CPU Meter Resources
xosview*cpu:                True
xosview*cpuUserColor:       FIELD1
xosview*cpuNiceColor:       FIELD2
xosview*cpuSystemColor:     FIELD3
xosview*cpuWaitColor:       FIELD4
xosview*cpuInterruptColor:  FIELD5
xosview*cpuSoftIntColor:    FIELD6
xosview*cpuStolenColor:     FIELD7
xosview*cpuGuestColor:      FIELD8
xosview*cpuNiceGuestColor:  FIELD9
xosview*cpuFreeColor:       FIELD0
xosview*cpuPriority:        2
xosview*cpuDecay:           True
xosview*cpuGraph:           False
xosview*cpuUsedFormat:	    percent
xosview*cpuUsedDecay:       True
xosview*cpuFormat:          auto   ! single, all, auto or both


! ** Memory Meter Resources
!    Not all of these are used by a specific platform.
xosview*mem:                True
xosview*memKernelColor:     LABEL2
xosview*memUsedColor:       FIELD1
xosview*memBufferColor:     FIELD2
xosview*memCacheColor:      FIELD3
xosview*memSwapCacheColor:  FIELD4
xosview*memFreeColor:       FIELD0
xosview*memTextColor:       FIELD5
xosview*memOtherColor:      FIELD6
xosview*memActiveColor:	    FIELD7
xosview*memInactiveColor:   FIELD8
xosview*memPriority:        10
xosview*memDecay:           False
xosview*memGraph:           False
xosview*memUsedFormat:	    autoscale


! ** Disk Meter Resources
xosview*disk:		    True
xosview*diskIdleColor:	    FIELD0
xosview*diskUsedColor:	    FIELD1
xosview*diskWriteColor:     FIELD2
xosview*diskReadColor:      FIELD3
xosview*diskBandwidth:      30000000  ! 30MB/s reasonable for IDE and SCSI
xosview*diskPriority:	    6
xosview*diskDecay:	    True
xosview*diskUsedFormat:     autoscale
xosview*diskUsedDecay:      True
xosview*diskGraph:	    False


! ** Swap Meter Resources
xosview*swap:               True
xosview*swapUsedColor:      FIELD1
xosview*swapFreeColor:      FIELD0
xosview*swapReservedColor:  FIELD2
xosview*swapPriority:       10
xosview*swapDecay:          False
xosview*swapGraph:          False
xosview*swapUsedFormat:	    autoscale
xosview*swapUsedDecay:      True

! ** Page Meter Resources
xosview*page:               True
xosview*pageBandwidth:      120
xosview*pageInColor:        FIELD1
xosview*pageOutColor:       FIELD2
xosview*pageIdleColor:      FIELD0
xosview*pagePriority:       5
xosview*pageDecay:          False
xosview*pageGraph:          False
xosview*pageUsedFormat:     autoscale
xosview*pageUsedDecay:      True


! Wifi Link meter
xosview*wlink:                False
xosview*wlinkBackground:      FIELD0
xosview*wlinkGoodColor:       FIELD1
xosview*wlinkPoorColor:       WARN
xosview*wlinkPoorValue:       39       ! scale is 0-70 this value is a guess
xosview*wlinkPriority:        8
xosview*wlinkDecay:           False
xosview*wlinkGraph:           False
xosview*wlinkUsedFormat:      autoscale
xosview*wlinkUsedDecay:       True


! ** Network Meter Resources
! The netBandwidth is now in bytes/sec because... I said so.
xosview*net:                True
!xosview*netBandwidth:      64000  ! for 64kbps ISDN
!xosview*netBandwidth:      57600  ! for up to 57.6kbps modems
!xosview*netBandwidth:      28800  ! for 28.8kbps modems (no comp) 14.4 (with)
xosview*netBandwidth:      6750000 ! 54Mbps 802.11g (wifi)
!xosview*netBandwidth:    10000000  ! for 10Mbit/s Ethernet
!xosview*netBandwidth:  100000000  ! for 100Mbit/s Ethernet
!xosview*netBandwidth: 1000000000  ! for 1Gbit/s Ethernet
xosview*netInColor:         FIELD1
xosview*netOutColor:        FIELD2
xosview*netBackground:      FIELD0
xosview*netPriority:        3
xosview*netDecay:           True
xosview*netGraph:           False
xosview*netUsedFormat:	    autoscale
xosview*netUsedDecay:       True
xosview*netIface:           False  ! Linux does not use. If others do not nuke


! ** Interrupt Rate Meter Resources
xosview*irqrate:	    True
xosview*irqrateUsedColor:   FIELD1
xosview*irqrateIdleColor:   FIELD0
xosview*irqratePriority:    4
xosview*irqratePeak:        100  # irq/sec
xosview*irqrateUsedFormat:  autoscale
xosview*irqrateUsedDecay:   True
xosview*irqrateDecay:       True
xosview*irqrateGraph:       False


! Interrupt Meter Resources
xosview*interrupts:         True
xosview*intOnColor:         FIELD1
xosview*intOffColor:        FIELD0
xosview*intPriority:	    4
xosview*intFormat:          auto    ! single, all, auto or both

! ** Battery Meter Resources
xosview*battery:            False
xosview*batteryLeftColor:   FIELD1
xosview*batteryUsedColor:   FIELD0
xosview*batteryPriority:    100      ! every 10 seconds
xosview*batteryUsedFormat:  percent
xosview*batteryChargeColor: FIELD2
xosview*batteryFullColor:   FIELD1
xosview*batteryLowColor:    WARN
xosview*batteryCritColor:   CRIT
xosview*batteryNoneColor:   DEAD


! ** Gfx Meter Resources ( Irix )
xosview*gfx:               True
xosview*gfxCritColor:      CRIT
xosview*gfxWarnColor:      WARN
xosview*gfxSwapColor:      FIELD1
xosview*gfxIdleColor:      FIELD0
xosview*gfxPriority:       1    ! dont change, until you know what you do
xosview*gfxCritThreshold:  120  ! swapbuffers/s per pipe
xosview*gfxWarnThreshold:  60   ! swapbuffers/s per pipe
xosview*gfxDecay:          False
xosview*gfxGraph:          False
xosview*gfxUsedFormat:	   autoscale


! Linux-only resources:


! ** Serial Meter Resources
!
! set 'True' for autodetection or give the baseaddress
! (autodetection may fail sometimes, if the port is
! locked by ppp/slip for example)
!xosview*serial0:            0x3f8
!xosview*serial1:            0x2f8
!xosview*serial2:            0x3e8
!xosview*serial3:            0x2e8
xosview*serial0:            False
xosview*serial1:            False
xosview*serial2:            False
xosview*serial3:            False
xosview*serial4:            False
xosview*serial5:            False
xosview*serial6:            False
xosview*serial7:            False
xosview*serial8:            False
xosview*serial9:            False
xosview*serialOnColor:      FIELD1
xosview*serialOffColor:     FIELD0
xosview*serialPriority:     1


! ** RAID
xosview*RAID:                 False
xosview*RAIDdevicecount:      1
xosview*RAIDdiskOnlineColor:  green
xosview*RAIDdiskFailureColor: CRIT
xosview*RAIDresyncdoneColor:  FIELD2
xosview*RAIDresynctodoColor:  FIELD1
xosview*RAIDresynccompleteColor:  FIELD0
xosview*RAIDPriority:         20
xosview*RAIDUsedFormat:       percent
xosview*horizontalMargin:      5  !
xosview*verticalMargin:        5  ! FIXME: no meter in name
xosview*verticalSpacing:       5  !

! Thermal Zone
xosview*tzone:                 True
xosview*tzoneForeGround:       FIELD1
xosview*tzoneBackground:       FIELD0
xosview*tzoneHotColor:         WARN
xosview*tzoneCritColor:        CRIT
xosview*tzoneHotTrip:          70
xosview*tzoneCritTrip:          84
xosview*tzonePeak:             100  ! peak of scale (total still in used)
xosview*tzonePriority:         10
xosview*tzoneDecay:            False
xosview*tzoneGraph:            False
xosview*tzoneUsedFormat:       autoscale
xosview*tzoneUsedDecay:        False

! ** Lms Temp
xosview*lmstemp:	False
xosview*lmstempActColor:       FIELD1
xosview*lmstempHighColor:      CRIT
xosview*lmstempIdleColor:      FIELD0
! xosview*lmstempHighest:	100
! default 100
xosview*lmstemp1:	temp
xosview*lmstemp2:	remote_temp
! for more sensors just add entries here
! note, there is currently no way to configure sensors with same
! name in different subdirs
! xosview*lmstempLabel1:	TMP
! labels for avove sensors (default = TMP)
xosview*lmstempLabel2:	RTMP
xosview*lmstempPriority: 20
xosview*lmstempUsedFormat: autoscale


! ** NFSD meter resources
xosview*NFSDStats:       False
xosview*NFSDStatCallsColor:     FIELD4
xosview*NFSDStatBadCallsColor:  FIELD3
xosview*NFSDStatUDPColor:       FIELD2
xosview*NFSDStatTCPColor:       FIELD1
xosview*NFSDStatIdleColor:      FIELD0
xosview*NFSDStatDecay:		True
xosview*NFSDStatGraph:		False
xosview*NFSDStatUsedFormat:	percent
xosview*NFSDStatUsedDecay:      False

! ** NFS meter resources
xosview*NFSStats:       False
xosview*NFSStatReTransColor:    FIELD3
xosview*NFSStatAuthRefrshColor: FIELD2
xosview*NFSStatCallsColor:      FIELD1
xosview*NFSStatIdleColor:       FIELD0
xosview*NFSStatDecay:		True
xosview*NFSStatGraph:		False
xosview*NFSStatUsedFormat:	percent

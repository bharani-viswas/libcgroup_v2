#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <search.h>

int cgrules_configure(void);

char *Acquisition_keys[] = {
"/usr/bin/xkbcomp",
"MammoServer.lnx",
"SupervisorServerProg",
"ApplicationManagerProg"
"GacSuifProxyProg",
"CameraServerProg",
"PositionerServerProg",
"CalibrationServerProg",
"GeneratorControllerProg",
"GacPoseidonControllerProg",
"IdcControllerProg",
"ConsoleController",
"ConsoleGUI",
"RecServer",
"ImProcServerProg",
"PlatformClient",
"TestDiagProg",
"dsa",
"netserver",
"nusmd",
"BiopsyToolEditor",
"BiopsyToolSvc",
"BiopsySvc",
"BiopsyCalibSvc",
"BiopsyBridge",
"NetLinkMonitor.lnx",
"InvokeSysCmd.lnx",
"CalibrationControllerProg",
"AcqSvcShutdown",
"PositionerPrefImpl",
"UnixCommandExecutor",
"CalTool",
"IQtool",
"WatchDogC",
"AcquisitionViewer",
"ProcWkfl",
"ImageWorkflow",
"ExamManager",
"DicomServer",
"DoseService"};

char *Background_keys[] = {
"/usr/sbin/logrotate",
"/usr/bin/python",
"/usr/bin/free",
"/etc/cron.daily/tmpwatch",
"/usr/bin/yum",
"/usr/bin/lsusb",
"/usr/sbin/zdump",
"/sbin/lvs",
"/usr/bin/timeout",
"/usr/bin/iostat",
"/usr/bin/pstack",
"/export/home/mammoApps/bin/pcimset",
"/export/home/mammoApps/scripts/usbreset.sh",
"/export/home/mammoApps/scripts/storeReconMACAdressonAxis.sh",
"/export/home/mammoApps/scripts/checkrsakey.sh",
"/export/home/mammoApps/scripts/log_volume_reconstruction.sh",
"/etc/rc.d/init.d/ReconNFSautoMount.sh",
"/export/home/mammoApps/scripts/synchAXISDateTime.sh",
"/opt/NAI/LinuxShield/bin/nails",
"/etc/aide/scripts/updateAIDE.sh",
"/export/home/insite/bin/iip-jweb",
"/export/home/insite/server/bin/iip-httpd",
"/export/home/sdc/nuevo/python/mediamgr/MediaManager.py",
"/export/home/sdc/nuevo/python/importer/ImageImporter.py",
"/export/home/sdc/senovision/bin/axisPCSensorDataPublisher",
"/export/home/sdc/senovision/scripts/monitorCPUTemp.sh",
"/export/home/sdc/senovision/scripts/cpuMemMonitor.sh",
"/export/home/sdc/senovision/scripts/monitorCPUClockSpeed.sh",
"/export/home/sdc/senovision/scripts/MonitorJavaProcesses.sh",
"/export/home/sdc/senovision/scripts/AxisPCMonitoring.sh",
"/export/home/sdc/senovision/bin/generateSnapshot",
"/export/home/sdc/senovision/scripts/snapshot/runSnapshotHandler.py",
"/export/home/sdc/senovision/scripts/dumpLogfiles.sh",
"/export/home/sdc/senovision/scripts/Snapshot_Generation",
"/export/home/sdc/senovision/scripts/Generate_Snapshot",
"/export/home/sdc/senovision/scripts/takeScreenShot.sh",
"/export/home/sdc/senovision/jars/TestAuditTrail.jar",
"/export/home/mammoApps/scripts/reportSwIntegrityCheck.sh",
"/etc/rc.d/rc5.d/S99PenMountsetUp",
"/etc/aide/scripts/checkAIDE.sh",
"/usr/share/gehc_security/csi/jetty/WebAdmin.sh",
"/usr/share/gehc_security/ea3/scripts/TimeoutClientTool.class",
"/export/home/mammoApps/scripts/reconconnectionspeed.sh",
"/export/home/sdc/senovision/bin/ConnectorUtil.lnx",
"/export/home/sdc/senovision/scripts/takePstack.sh",
"/export/home/mammoApps/scripts/updateAntivirusAxisToRecon.sh",
"/export/home/sdc/senovision/scripts/dst.sh",
"/export/home/sdc/senovision/scripts/CoreCleanup.pl",
"/export/home/sdc/nuevo/scripts/util/vacuum.sh",
"/export/home/sdc/senovision/scripts/logCleaner.sh",
"/export/home/mammoApps/scripts/dod_check_recon_recon_user_password.sh",
"/export/home/mammoApps/scripts/dod_check_recon_root_user_password.sh",
"/export/home/mammoApps/scripts/dod_check_recon_firewall_status.sh",
"/export/home/mammoApps/scripts/dod_check_recon_started.sh",
"/export/home/sdc/senovision/scripts/gatherstats.bash",
"/export/home/sdc/senovision/scripts/gatherstats_cpu.bash",
"/export/home/mammoApps/scripts/checkAxisReconAntivirusStatus.sh",
"/export/home/mammoApps/scripts/checkChangeHostnameRecon.sh",
"/export/home/mammoApps/scripts/checkChangeIpRecon.sh",
"/export/home/mammoApps/scripts/checkCurrentSshSessionRecon.sh",
"/export/home/insite/bin/prune_httpd_log",
"/export/home/sdc/senovision/scripts/mem_usage.sh",
"/export/home/sdc/senovision/scripts/checkForMalware.sh",
"/opt/STIGeye/scriplets/RHEL/packageMgmtTool.sh",
"/opt/STIGeye/scriplets/RHEL/aide.sh",
"/export/home/sdc/nuevo/scripts/util/cleanUpLogs.csh",
"/export/home/sdc/filmComposer/scripts/CronJob.sh",
"/export/home/gac/scripts/CalibrationLogging.sh",
"/export/home/idc/bin/turnCorrection.sh",
"/opt/McAfee/ens/esp/bin/mfeespd",
"/opt/McAfee/ens/tp/bin/mfetpd"
};


struct hsearch_data * cgrules_slice_dict;
 
int cgrules_configure(void)
{
        int Acquisition_slice_entries = sizeof(Acquisition_keys)/sizeof(Acquisition_keys[0]);
        int Background_slice_entries = sizeof(Background_keys)/sizeof(Background_keys[0]);
        ENTRY e, *ep;
        int i;
        int cgrules_entries = Background_slice_entries+Acquisition_slice_entries;
	cgrules_slice_dict = calloc(1,sizeof(struct hsearch_data));
	hcreate_r(cgrules_entries,cgrules_slice_dict);

        for (i=0;i<Acquisition_slice_entries;i++)
        {
                //Keys represent the binaries and data (values) represent the slice names
                        e.key = Acquisition_keys[i];
                        e.data = "Acquisition.slice";
                        hsearch_r(e, ENTER,&ep,cgrules_slice_dict);
                        if (ep == NULL) {
                                fprintf(stderr, "entry failed\n");
                                return -1;
                        }

        }

        for (i = 0; i < Background_slice_entries; i++)
        {
                //Keys represent the binaries and data (values) represent the slice names
                        e.key = Background_keys[i];
                        e.data = "Background.slice";
                        hsearch_r(e, ENTER,&ep,cgrules_slice_dict);                        
			if (ep == NULL) {
                                fprintf(stderr, "entry failed\n");
                                return -1;
                        }


        }

return 1;
}


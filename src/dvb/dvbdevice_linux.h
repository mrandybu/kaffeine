/*
 * dvbdevice_linux.h
 *
 * Copyright (C) 2007-2011 Christoph Pfister <christophpfister@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DVBDEVICE_LINUX_H
#define DVBDEVICE_LINUX_H

#include <QThread>
#include "dvbbackenddevice.h"
#include "dvbcam_linux.h"

extern "C" {
  #include <libdvbv5/dvb-file.h>
  #include <libdvbv5/dvb-demux.h>
  #include <libdvbv5/dvb-v5-std.h>
  #include <libdvbv5/dvb-scan.h>
}

class DvbLinuxDevice : public QThread, public DvbBackendDevice
{
public:
	explicit DvbLinuxDevice(QObject *parent);
	~DvbLinuxDevice();

	bool isReady() const;
	void startDevice(const QString &deviceId_);
	void startCa();
	void stopCa();
	void stopDevice();
	void enableDvbDump();
	QString getDeviceId();
	QString getFrontendName();

	QString caPath;
	QString caUdi;
	QString demuxPath;
	QString demuxUdi;
	QString dvrPath;
	QString dvrUdi;
	int adapter;
	int index;
	int numDemux;
	struct dvb_v5_fe_parms *dvbv5_parms;
	QString frontendPath;
	QString frontendUdi;

protected:
	TransmissionTypes getTransmissionTypes();
	Capabilities getCapabilities();
	void setFrontendDevice(DvbFrontendDevice *frontend_);
	void setDeviceEnabled(bool enabled_);
	bool acquire();
	bool setHighVoltage(int higherVoltage);
	bool sendMessage(const char *message, int length);
	bool sendBurst(SecBurst burst);
	bool satSetup(QString lnbModel, int satNumber, int bpf);
	bool tune(const DvbTransponder &transponder); // discards obsolete data
	bool getProps(DvbTransponder &transponder);
	bool isTuned();
	float getFrqMHz();
	float getSignal(Scale &scale);
	float getSnr(DvbBackendDevice::Scale &scale);
	bool addPidFilter(int pid);
	void removePidFilter(int pid);
	void startDescrambling(const QByteArray &pmtSectionData);
	void stopDescrambling(int serviceId);
	void release();

private:
	void startDvr();
	void stopDvr();
	void run();

	bool ready;
	QString deviceId;
	QString frontendName;
	TransmissionTypes transmissionTypes;
	Capabilities capabilities;
	DvbFrontendDevice *frontend;
	bool enabled;
	QMap<int, int> dmxFds;

	float freqMHz;

	int verbose;
	int frontendFd;
	int dvrFd;
	int dvrPipe[2];
	DvbDataBuffer dvrBuffer;

	DvbLinuxCam cam;
};

class DvbDeviceMonitor;
class DvbLinuxDeviceManager : public QObject
{
	Q_OBJECT
public:
	explicit DvbLinuxDeviceManager(QObject *parent);
	~DvbLinuxDeviceManager();

	void componentAdded(QString node, int adapter, int index);
	void componentRemoved(QString node, int adapter, int index);
public slots:
	void doColdPlug();

signals:
	void requestBuiltinDeviceManager(QObject *&bultinDeviceManager);
	void deviceAdded(DvbBackendDevice *device);
	void deviceRemoved(DvbBackendDevice *device);

private slots:
	void componentAdded(const QString &udi);
	void componentRemoved(const QString &udi);

private:
	int readSysAttr(const QString &path);

	QMap<int, DvbLinuxDevice *> devices;
	QMap<QString, DvbLinuxDevice *> udis;
	class DvbDeviceMonitor *monitor;
};

#endif /* DVBDEVICE_LINUX_H */

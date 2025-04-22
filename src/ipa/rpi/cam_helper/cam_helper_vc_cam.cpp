/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2019, Raspberry Pi Ltd
 *
 * camera helper for imx219 sensor
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <libcamera/base/log.h>

#include <string>


#include "cam_helper.h"


using namespace RPiController;
using namespace libcamera;
LOG_DEFINE_CATEGORY(CamHelperImxVCCamera)



/*
 * We care about one gain register and a pair of exposure registers. Their I2C
 * addresses from the Sony IMX219 datasheet:
 */
constexpr uint32_t gainReg = 0x157;
constexpr uint32_t expHiReg = 0x010A;
constexpr uint32_t expLoReg = 0x109;
constexpr uint32_t frameLengthHiReg = 0x160;
constexpr uint32_t frameLengthLoReg = 0x161;
constexpr uint32_t lineLengthHiReg = 0x162;
constexpr uint32_t lineLengthLoReg = 0x163;
constexpr std::initializer_list<uint32_t> registerList [[maybe_unused]]
	= { expHiReg, expLoReg, gainReg, frameLengthHiReg, frameLengthLoReg,
	    lineLengthHiReg, lineLengthLoReg };

class CamHelperImxVCCamera : public CamHelper
{

public:
	CamHelperImxVCCamera();
	uint32_t gainCode(double gain) const override;
	double gain(uint32_t gainCode) const override;
	unsigned int mistrustFramesModeSwitch() const override;
	bool sensorEmbeddedDataPresent() const override;

private:
	/*
	 * Smallest difference between the frame length and integration time,
	 * in units of lines.
	 */
	static constexpr int frameIntegrationDiff = 4;

	// void populateMetadata(const MdParser::RegisterMap &registers,
	// 		      Metadata &metadata) const override;
};

CamHelperImxVCCamera::CamHelperImxVCCamera()
#if ENABLE_EMBEDDED_DATA
	: CamHelper(std::make_unique<MdParserSmia>(registerList), frameIntegrationDiff)
#else
	: CamHelper({}, frameIntegrationDiff)
#endif
{
}

uint32_t CamHelperImxVCCamera::gainCode(double gain) const
{
	return (uint32_t)gain;
}

double CamHelperImxVCCamera::gain(uint32_t gainCode) const
{
	return gainCode;
}

unsigned int CamHelperImxVCCamera::mistrustFramesModeSwitch() const
{
	/*
	 * For reasons unknown, we do occasionally get a bogus metadata frame
	 * at a mode switch (though not at start-up). Possibly warrants some
	 * investigation, though not a big deal.
	 */
	return 1;
}

bool CamHelperImxVCCamera::sensorEmbeddedDataPresent() const
{
	return false;
}

// void CamHelperImxVCCamera::populateMetadata(const MdParser::RegisterMap &registers,
// 				       Metadata &metadata) const
// {
// 	DeviceStatus deviceStatus;

// 	deviceStatus.lineLength = lineLengthPckToDuration(registers.at(lineLengthHiReg) * 256 +
// 							  registers.at(lineLengthLoReg));
// 	deviceStatus.exposureTime = exposure(registers.at(expHiReg) * 256 + registers.at(expLoReg),
// 					     deviceStatus.lineLength);
// 	LOG(CamHelperImxVCCamera, Error) << "Exposure time: " << deviceStatus.exposureTime.count() << " us";
// 	deviceStatus.analogueGain = gain(registers.at(gainReg));
// 	deviceStatus.frameLength = registers.at(frameLengthHiReg) * 256 + registers.at(frameLengthLoReg);

// 	metadata.set("device.status", deviceStatus);
// }

static CamHelper *create()
{
	return new CamHelperImxVCCamera();
}

static RegisterCamHelper reg("vc_mipi_camera", &create);

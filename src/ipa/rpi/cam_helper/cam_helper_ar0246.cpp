/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * camera helper for ar0246 sensor
 */

#include <algorithm>
#include <cmath>

#include "cam_helper.h"

using namespace RPiController;

class CamHelperAr0246 : public CamHelper
{
public:
	CamHelperAr0246();
	uint32_t gainCode(double gain) const override;
	double gain(uint32_t gainCode) const override;
	unsigned int hideFramesStartup() const override;
	unsigned int hideFramesModeSwitch() const override;

private:
	/*
	 * Smallest difference between the frame length and integration time,
	 * in units of lines.
	 */
	static constexpr int frameIntegrationDiff = 1;
	/*
	 * The driver maps V4L2_CID_ANALOGUE_GAIN onto SENSOR_GAIN (R0x5900),
	 * an index into the sensor's gain table: code 0..111 selects
	 * 0..41.625 dB in 0.375 dB steps (the signal doubles every 16 codes).
	 */
	static constexpr double gainStepDb = 0.375;
	static constexpr uint32_t gainCodeMax = 111;
};

CamHelperAr0246::CamHelperAr0246()
	: CamHelper({}, frameIntegrationDiff)
{
}

uint32_t CamHelperAr0246::gainCode(double gain) const
{
	if (gain <= 1.0)
		return 0;
	long code = std::lround(20.0 * std::log10(gain) / gainStepDb);
	return std::min(static_cast<uint32_t>(code), gainCodeMax);
}

double CamHelperAr0246::gain(uint32_t gainCode) const
{
	return std::pow(10.0, gainCode * gainStepDb / 20.0);
}

unsigned int CamHelperAr0246::hideFramesStartup() const
{
	/* Allow the exposure and gain values to settle after streaming starts. */
	return 1;
}

unsigned int CamHelperAr0246::hideFramesModeSwitch() const
{
	return 1;
}

static CamHelper *create()
{
	return new CamHelperAr0246();
}

static RegisterCamHelper reg("ar0246", &create);

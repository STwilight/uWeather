#ifndef INCLUDE_NTPCLIENT_H_
#define INCLUDE_NTPCLIENT_H_

class ntpClientClass
{
public:
	ntpClientClass()
	{
		ntpcp = new NtpClient("pool.ntp.org", 30, NtpTimeResultDelegate(&ntpClientClass::ntpResult, this));
	};

	void ntpResult(NtpClient& client, time_t ntpTime)
	{
		SystemClock.setTime(ntpTime, eTZ_UTC);
	}

private:
	NtpClient *ntpcp;
};

#endif

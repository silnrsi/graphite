using System;

namespace NGraphite
{
	[Flags]
	public enum LogMask
	{
		LOG_NONE = 0x0,
		LOG_FACE = 0x01,
		LOG_SEGMENT = 0x02,
		LOG_PASS = 0x04,
		LOG_CACHE = 0x08,
		
		LOG_OPCODE = 0x80,
		LOG_ALL = 0xFF
	}
}


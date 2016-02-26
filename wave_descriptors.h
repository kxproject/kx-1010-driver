/* 1010 PCI/PCIe Audio Driver
   Copyright (c) Eugene Gavrilov, 2002-2016

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
   * You should have received a copy of the GNU General Public License
   * along with this program; if not, write to the Free Software
   * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#pragma once

static KSDATARANGE PinDataRangesBridge[2] =
{
   {
      sizeof(KSDATARANGE),
      0,
      0,
      0,
      STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
      STATICGUIDOF(KSDATAFORMAT_SUBTYPE_ANALOG),
      STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
   },
   {
      sizeof(KSDATARANGE),
      0,
      0,
      0,
      STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
      STATICGUIDOF(KSDATAFORMAT_SUBTYPE_AC3_AUDIO),
      STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
   }
};


// -------- Recording
static KSDATARANGE_AUDIO PinDataRangesStreamRecording[12] =
{
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        44100,  // Minimum rate.
        44100   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        44100,  // Minimum rate.
        44100   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        48000,  // Minimum rate.
        48000   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        48000,  // Minimum rate.
        48000   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        88200,  // Minimum rate.
        88200   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        88200,  // Minimum rate.
        88200   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        96000,  // Minimum rate.
        96000   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        96000,  // Minimum rate.
        96000   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        176400, // Minimum rate.
        176400  // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        176400,  // Minimum rate.
        176400   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        192000, // Minimum rate.
        192000  // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        192000, // Minimum rate.
        192000  // Maximum rate.
    }
};

static KSDATARANGE_AUDIO PinDataRangesStreamPlayback[12] =
{
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        44100,  // Minimum rate.
        44100   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        44100,  // Minimum rate.
        44100   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        48000,  // Minimum rate.
        48000   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        48000,  // Minimum rate.
        48000   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        88200,  // Minimum rate.
        88200   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        88200,  // Minimum rate.
        88200   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        96000,  // Minimum rate.
        96000   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        96000,  // Minimum rate.
        96000   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        176400, // Minimum rate.
        176400  // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        176400,  // Minimum rate.
        176400   // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        16,     // Minimum number of bits per sample.
        16,     // Maximum number of bits per channel.
        192000, // Minimum rate.
        192000  // Maximum rate.
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        2,      // Max number of channels.
        24,     // Minimum number of bits per sample.
        32,     // Maximum number of bits per channel.
        192000, // Minimum rate.
        192000  // Maximum rate.
    }
};

static KSDATARANGE_AUDIO PinDataRangesStreamPlaybackAC3[1] =
{
  {
    {
      sizeof(KSDATARANGE_AUDIO),
      0,
      0,
      0,
      STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
      STATICGUIDOF(KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL),
      STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
    },
    2,      // Max number of channels.
    16,     // Minimum number of bits per sample.
    16,     // Maximum number of bits per channel.
    48000,  // Minimum rate.
    48000   // Maximum rate.
  }
};

static PKSDATARANGE PinDataRangePointersPlayback[12] =
{
    PKSDATARANGE(&PinDataRangesStreamPlayback[0]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[1]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[2]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[3]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[4]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[5]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[6]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[7]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[8]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[9]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[10]),
    PKSDATARANGE(&PinDataRangesStreamPlayback[11])
};


static PKSDATARANGE PinDataRangePointersPlaybackAC3[1] =
{
    PKSDATARANGE(&PinDataRangesStreamPlaybackAC3[0])
};

static PKSDATARANGE PinDataRangePointersRecording[12] =
{
    PKSDATARANGE(&PinDataRangesStreamRecording[0]),
    PKSDATARANGE(&PinDataRangesStreamRecording[1]),
    PKSDATARANGE(&PinDataRangesStreamRecording[2]),
    PKSDATARANGE(&PinDataRangesStreamRecording[3]),
    PKSDATARANGE(&PinDataRangesStreamRecording[4]),
    PKSDATARANGE(&PinDataRangesStreamRecording[5]),
    PKSDATARANGE(&PinDataRangesStreamRecording[6]),
    PKSDATARANGE(&PinDataRangesStreamRecording[7]),
    PKSDATARANGE(&PinDataRangesStreamRecording[8]),
    PKSDATARANGE(&PinDataRangesStreamRecording[9]),
    PKSDATARANGE(&PinDataRangesStreamRecording[10]),
    PKSDATARANGE(&PinDataRangesStreamRecording[11])
};

static PKSDATARANGE PinDataRangePointersBridge[] =
{
    &PinDataRangesBridge[0]
};


static PKSDATARANGE PinDataRangePointersBridgeSPDIF[] =
{
    &PinDataRangesBridge[1]
};

// MiniportPins
// List of pins.

static PCPIN_DESCRIPTOR MiniportPins[] =
{
    // Wave Out Streaming Pin (Renderer) - WAVE_WAVEOUT_SOURCE
#if defined(WAVE_WAVEOUT_SOURCE)
    {
        1,1,0,
        NULL,
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(PinDataRangePointersPlayback),
            PinDataRangePointersPlayback,
            KSPIN_DATAFLOW_IN,
            KSPIN_COMMUNICATION_SINK,
            &KSCATEGORY_AUDIO,
            NULL,	// Name
            0
        }
    },
#endif    
    // Wave Out Bridge Pin (Renderer) - WAVE_WAVEOUT_DEST
#if defined(WAVE_WAVEOUT_DEST)
    {
        0,0,0,
        NULL,
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(PinDataRangePointersBridge),
            PinDataRangePointersBridge,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_NONE,
            &KSCATEGORY_AUDIO,
            NULL,	// Name
            0
        }
    },
#endif

    // Wave In Streaming Pin (Capture) - WAVE_WAVEIN_SOURCE
#if defined(WAVE_WAVEIN_SOURCE)
    {
        1,1,0,
        NULL,
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(PinDataRangePointersRecording),
            PinDataRangePointersRecording,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_SINK,
            &KSCATEGORY_AUDIO,
            &KSAUDFNAME_RECORDING_CONTROL, // this name shows up as the recording panel name in SoundVol.
            0
        }
    },
#endif

    // Wave In Bridge Pin (Capture - To Topology) - WAVE_WAVEIN_DEST
#if defined(WAVE_WAVEIN_DEST)
    {
        0,0,0,
        NULL,
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(PinDataRangePointersBridge),
            PinDataRangePointersBridge,
            KSPIN_DATAFLOW_IN,
            KSPIN_COMMUNICATION_NONE,
            &KSCATEGORY_AUDIO,
            NULL, // Name
            0
        }
    },
#endif

    // AC3 SPDIF Digital Output - WAVE_SPDIF_AC3
#if defined(WAVE_SPDIF_AC3)
    {
        1,1,0,
    	NULL,
    	{
    		0,
    		NULL,
    		0,
    		NULL,
    		SIZEOF_ARRAY(PinDataRangePointersPlaybackAC3),
            PinDataRangePointersPlaybackAC3,
	        KSPIN_DATAFLOW_IN,
	        KSPIN_COMMUNICATION_SINK,
	        &KSCATEGORY_AUDIO,
	        NULL,	// Name
	        0
        }
    },
#endif

    // SPDIF Out Bridge Pin - WAVE_SPDIF
#if defined(WAVE_SPDIF)
    {
        0,0,0,
        NULL,
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(PinDataRangePointersBridgeSPDIF),
            PinDataRangePointersBridgeSPDIF,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_NONE,
            &KSNODETYPE_SPDIF_INTERFACE,
            NULL,	// Name
            0
        }
    }
#endif

};

static PCPROPERTY_ITEM NodeProperties[] =
{
    { //
        &KSPROPSETID_DRIVER_Private,
        KSPROPERTY_DRIVER_WAVE,
        KSPROPERTY_TYPE_GET|KSPROPERTY_TYPE_SET,
        WaveStream::PropertyPrivate
    }
};

DEFINE_PCAUTOMATION_TABLE_PROP (NodeAutomation,NodeProperties);

// TopologyNodes
// List of nodes.

static PCNODE_DESCRIPTOR MiniportNodes[] =
{
	// WAVE_DAC
    {
        0,                      // Flags
        &NodeAutomation,		// AutomationTable
        &KSNODETYPE_DAC,        // Type
        NULL					// Name 
    },
    // WAVE_ADC
    {
        0,                      // Flags
        &NodeAutomation,		// AutomationTable
        &KSNODETYPE_ADC,        // Type
        NULL					// Name 
    }
};

enum
{
 WAVE_DAC=0,
 WAVE_ADC
};

// MiniportConnections
// List of connections.

static PCCONNECTION_DESCRIPTOR MiniportConnections[] =
{
    { PCFILTER_NODE,  WAVE_WAVEOUT_SOURCE,  WAVE_DAC,         	1 },    					// Stream in to DAC.
    { WAVE_DAC,       0,  					PCFILTER_NODE,    	WAVE_WAVEOUT_DEST },    	// DAC to Bridge.

#if defined(WAVE_SPDIF_AC3) && defined(WAVE_SPDIF)
    { PCFILTER_NODE,  WAVE_SPDIF_AC3,		PCFILTER_NODE,	    WAVE_SPDIF },
#endif

    { PCFILTER_NODE,  WAVE_WAVEIN_DEST,   	WAVE_ADC,         	1 },    // Bridge in to ADC.
    { WAVE_ADC,       0,  					PCFILTER_NODE,    	WAVE_WAVEIN_SOURCE }    // ADC to stream pin (capture).
};


static PCPROPERTY_ITEM FilterProperties[] =
{
    { //
        &KSPROPSETID_DRIVER_Private,
        KSPROPERTY_DRIVER_WAVE,
        KSPROPERTY_TYPE_GET|KSPROPERTY_TYPE_SET,
        WaveStream::PropertyPrivate
    },
    {
    	&KSPROPSETID_Pin,
        KSPROPERTY_PIN_PROPOSEDATAFORMAT,
        KSPROPERTY_TYPE_GET|KSPROPERTY_TYPE_SET/*|KSEVENT_TYPE_BASICSUPPORT*/,
        Wave::ProposeFormatHandler
    }
};

// Declare an event item for format change
static PCEVENT_ITEM FilterEvents[] =
{
  {
    &KSEVENTSETID_PinCapsChange,   // Something on the pin changed!
    KSEVENT_PINCAPS_FORMATCHANGE,  // Format changes
    KSEVENT_TYPE_ENABLE | KSEVENT_TYPE_BASICSUPPORT,
    Wave::FormatChangeHandler
  }
};


DEFINE_PCAUTOMATION_TABLE_PROP_EVENT (FilterAutomation,FilterProperties,FilterEvents);


// MiniportFilterDescriptor
// Complete miniport description.

static PCFILTER_DESCRIPTOR MiniportFilterDescriptor =
{
    0,                                  // Version
    &FilterAutomation,                  // AutomationTable
    sizeof(PCPIN_DESCRIPTOR),           // PinSize
    SIZEOF_ARRAY(MiniportPins),         // PinCount
    MiniportPins,                       // Pins
    sizeof(PCNODE_DESCRIPTOR),          // NodeSize
    SIZEOF_ARRAY(MiniportNodes),        // NodeCount
    MiniportNodes,                      // Nodes
    SIZEOF_ARRAY(MiniportConnections),  // ConnectionCount
    MiniportConnections,                // Connections
    0,                                  // CategoryCount
    NULL                                // Categories
};

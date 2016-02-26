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

#include "common.h"
#include "debug.h"

#include "property.h"
#include "topology.h"
#include "dsp.h"

#define KSPROPERTY_TYPE_ALL         KSPROPERTY_TYPE_BASICSUPPORT | \
                                    KSPROPERTY_TYPE_GET | \
                                    KSPROPERTY_TYPE_SET

#pragma code_seg("PAGE")
NTSTATUS Property::PropertyCpuResources(IN PPCPROPERTY_REQUEST   PropertyRequest)
{
	PAGED_CODE();

    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

    if (PropertyRequest->Verb & KSPROPERTY_TYPE_GET)
    {
        ntStatus = ValidatePropertyParams(PropertyRequest, sizeof(ULONG));
        if (NT_SUCCESS(ntStatus))
        {
            *(PLONG(PropertyRequest->Value)) = KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU;
            PropertyRequest->ValueSize = sizeof(LONG);
        }
    }
    else
     if (PropertyRequest->Verb & KSPROPERTY_TYPE_BASICSUPPORT)
     {
         ntStatus = PropertyHandler_BasicSupport( PropertyRequest, KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT, VT_ILLEGAL, FALSE);
     }

    return ntStatus;
}

#pragma code_seg("PAGE")
NTSTATUS Property::PropertyMute(IN  PPCPROPERTY_REQUEST     PropertyRequest)
{
    PAGED_CODE();

    NTSTATUS                    ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    LONG                        lChannel;
    PBOOL                       pfMute;

    if (PropertyRequest->Verb & KSPROPERTY_TYPE_BASICSUPPORT)
    {
        ntStatus = PropertyHandler_BasicSupport(PropertyRequest,KSPROPERTY_TYPE_ALL,VT_BOOL,TRUE);
    }
    else
    {
        ntStatus = ValidatePropertyParams(PropertyRequest, sizeof(BOOL), sizeof(LONG));

        if (NT_SUCCESS(ntStatus))
        {
            lChannel = * PLONG (PropertyRequest->Instance);
            pfMute   = PBOOL (PropertyRequest->Value);

            Topology *obj = ((Topology *)((PMINIPORTTOPOLOGY)(PropertyRequest->MajorTarget)));

            if(!valid_object(obj,Topology)) { debug("!! PropertyMute: invalid major target\n"); return STATUS_INVALID_PARAMETER; }

            if (PropertyRequest->Verb & KSPROPERTY_TYPE_GET)
            {
            	ntStatus=obj->GetMute(PropertyRequest->Node,lChannel, pfMute);
                PropertyRequest->ValueSize = sizeof(BOOL);
            }
            else if (PropertyRequest->Verb & KSPROPERTY_TYPE_SET)
            {
            	ntStatus=obj->SetMute(PropertyRequest->Node, lChannel, *pfMute);
            }
        }
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
NTSTATUS Property::PropertyVolume(IN  PPCPROPERTY_REQUEST PropertyRequest)
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    LONG     lChannel;
    PULONG   pulVolume;

    if (PropertyRequest->Verb & KSPROPERTY_TYPE_BASICSUPPORT)
    {
        ntStatus = PropertyHandlerBasicSupportVolume(PropertyRequest);
    }
    else
    {
        ntStatus = ValidatePropertyParams(PropertyRequest, sizeof(ULONG), sizeof(LONG));

        if (NT_SUCCESS(ntStatus))
        {
            lChannel = * (PLONG (PropertyRequest->Instance));
            pulVolume = PULONG (PropertyRequest->Value);

            Topology *obj = ((Topology *)((PMINIPORTTOPOLOGY)(PropertyRequest->MajorTarget)));

            if(!valid_object(obj,Topology)) { debug("!! PropertyVolume: invalid major target\n"); return STATUS_INVALID_PARAMETER; }

            if (PropertyRequest->Verb & KSPROPERTY_TYPE_GET)
            {
            	ntStatus=obj->GetVolume(PropertyRequest->Node,lChannel,pulVolume);
                PropertyRequest->ValueSize = sizeof(ULONG);                
            }
            else if (PropertyRequest->Verb & KSPROPERTY_TYPE_SET)
            {
            	ntStatus=obj->SetVolume(PropertyRequest->Node,lChannel,*pulVolume);
            }
        }
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
NTSTATUS Property::ValidatePropertyParams(IN PPCPROPERTY_REQUEST PropertyRequest, IN ULONG cbSize,IN ULONG cbInstanceSize)
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    if (PropertyRequest && cbSize)
    {
        // If the caller is asking for ValueSize.
        if (PropertyRequest->ValueSize == 0) 
        {
            PropertyRequest->ValueSize = cbSize;
            ntStatus = STATUS_BUFFER_OVERFLOW;
        }
        // If the caller passed an invalid ValueSize.
        else if (PropertyRequest->ValueSize < cbSize)
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else if (PropertyRequest->InstanceSize < cbInstanceSize)
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }

        // If all parameters are OK.
        // 
        else if (PropertyRequest->ValueSize == cbSize)
        {
            if (PropertyRequest->Value)
            {
                ntStatus = STATUS_SUCCESS;
                //
                // Caller should set ValueSize, if the property 
                // call is successful.
                //
            }
        }
    }
    else
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    
    // Clear the ValueSize if unsuccessful.
    //
    if (PropertyRequest &&
        STATUS_SUCCESS != ntStatus &&
        STATUS_BUFFER_OVERFLOW != ntStatus)
    {
        PropertyRequest->ValueSize = 0;
    }

    return ntStatus;
}


#pragma code_seg("PAGE")
NTSTATUS Property::PropertyHandler_BasicSupport(IN PPCPROPERTY_REQUEST PropertyRequest,IN ULONG Flags,IN DWORD PropTypeSetId,BOOL Multichannel)
//  Default basic support handler. Basic processing depends on the size of data.
//  For ULONG it only returns Flags. For KSPROPERTY_DESCRIPTION, the structure   
//  is filled.
{
    PAGED_CODE();

    ASSERT(Flags & KSPROPERTY_TYPE_BASICSUPPORT);

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	Topology *obj = ((Topology *)((PMINIPORTTOPOLOGY)PropertyRequest->MajorTarget));
    if(!valid_object(obj,Topology)) { debug("!! PropertyHandler_BasicSupport: invalid major target\n"); return STATUS_INVALID_PARAMETER; }
    int num_channels=obj->get_n_channels();

    ULONG cbFullProperty = sizeof(KSPROPERTY_DESCRIPTION) 
    				+ (Multichannel?(sizeof(KSPROPERTY_MEMBERSHEADER) + sizeof(KSPROPERTY_STEPPING_LONG)*num_channels):0);

    if (PropertyRequest->ValueSize >= sizeof(KSPROPERTY_DESCRIPTION))
    {
        // if return buffer can hold a KSPROPERTY_DESCRIPTION, return it
        //
        PKSPROPERTY_DESCRIPTION PropDesc = PKSPROPERTY_DESCRIPTION(PropertyRequest->Value);

        PropDesc->AccessFlags       = Flags;
        PropDesc->DescriptionSize   = cbFullProperty;

        if  (PropTypeSetId != VT_ILLEGAL)
        {
            PropDesc->PropTypeSet.Set   = KSPROPTYPESETID_General;
            PropDesc->PropTypeSet.Id    = PropTypeSetId;
        }
        else
        {
            PropDesc->PropTypeSet.Set   = GUID_NULL;
            PropDesc->PropTypeSet.Id    = 0;
        }

        PropDesc->PropTypeSet.Flags = 0;
        PropDesc->MembersListCount  = Multichannel?1:0;
        PropDesc->Reserved          = 0;

        // if return buffer can also hold a range description, return it too
        if(PropertyRequest->ValueSize >= cbFullProperty && Multichannel)
        {
            // fill in the members header
            PKSPROPERTY_MEMBERSHEADER Members = PKSPROPERTY_MEMBERSHEADER(PropDesc + 1);

            Members->MembersFlags   = KSPROPERTY_MEMBER_STEPPEDRANGES;
            Members->MembersSize    = sizeof(KSPROPERTY_STEPPING_LONG);
            Members->MembersCount   = num_channels;
            Members->Flags          = KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL;

            // fill in the stepped range
            PKSPROPERTY_STEPPING_LONG Range = PKSPROPERTY_STEPPING_LONG(Members + 1);

            for(int i=0;i<num_channels;i++)
            {
            	Range->Bounds.SignedMaximum = 1;
            	Range->Bounds.SignedMinimum = 0;
            	Range->SteppingDelta        = 1;
            	Range->Reserved             = 0;
            }

            // set the return value size
            PropertyRequest->ValueSize = cbFullProperty;
        }
        else
        {
        	PropertyRequest->ValueSize = sizeof(KSPROPERTY_DESCRIPTION);
        }

        ntStatus = STATUS_SUCCESS;
    } 
    else if (PropertyRequest->ValueSize >= sizeof(ULONG))
    {
        // if return buffer can hold a ULONG, return the access flags
        //
        *(PULONG(PropertyRequest->Value)) = Flags;

        PropertyRequest->ValueSize = sizeof(ULONG);
        ntStatus = STATUS_SUCCESS;                    
    }
    else
    {
        PropertyRequest->ValueSize = 0;
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
NTSTATUS Property::PropertyHandlerBasicSupportVolume(IN  PPCPROPERTY_REQUEST     PropertyRequest)
{
    PAGED_CODE();

	Topology *obj = ((Topology *)((PMINIPORTTOPOLOGY)PropertyRequest->MajorTarget));
    if(!valid_object(obj,Topology)) { debug("!! PropertyHandlerBasicSupportVolume: invalid major target\n"); return STATUS_INVALID_PARAMETER; }
    int num_channels=obj->get_n_channels();

    NTSTATUS                    ntStatus = STATUS_SUCCESS;
    ULONG                       cbFullProperty = sizeof(KSPROPERTY_DESCRIPTION) + sizeof(KSPROPERTY_MEMBERSHEADER) + sizeof(KSPROPERTY_STEPPING_LONG)*num_channels;

    if (PropertyRequest->ValueSize >= (sizeof(KSPROPERTY_DESCRIPTION)))
    {
        PKSPROPERTY_DESCRIPTION PropDesc = PKSPROPERTY_DESCRIPTION(PropertyRequest->Value);

        PropDesc->AccessFlags       = KSPROPERTY_TYPE_ALL;
        PropDesc->DescriptionSize   = cbFullProperty;
        PropDesc->PropTypeSet.Set   = KSPROPTYPESETID_General;
        PropDesc->PropTypeSet.Id    = VT_I4;
        PropDesc->PropTypeSet.Flags = 0;
        PropDesc->MembersListCount  = 1;
        PropDesc->Reserved          = 0;

        // if return buffer can also hold a range description, return it too
        if(PropertyRequest->ValueSize >= cbFullProperty)
        {
            // fill in the members header
            PKSPROPERTY_MEMBERSHEADER Members = 
                PKSPROPERTY_MEMBERSHEADER(PropDesc + 1);

            Members->MembersFlags   = KSPROPERTY_MEMBER_STEPPEDRANGES;
            Members->MembersSize    = sizeof(KSPROPERTY_STEPPING_LONG);
            Members->MembersCount   = num_channels;
            Members->Flags          = KSPROPERTY_MEMBER_FLAG_BASICSUPPORT_MULTICHANNEL;

            // fill in the stepped range
            PKSPROPERTY_STEPPING_LONG Range = PKSPROPERTY_STEPPING_LONG(Members + 1);

            for(int i=0;i<num_channels;i++)
            {
            	Range->Bounds.SignedMaximum = DSP::MAX_VOLUME;
            	Range->Bounds.SignedMinimum = DSP::MIN_VOLUME;
            	Range->SteppingDelta        = DSP::VOLUME_STEP;
            	Range->Reserved             = 0;
            }

            // set the return value size
            PropertyRequest->ValueSize = cbFullProperty;
        } 
        else
        {
            PropertyRequest->ValueSize = sizeof(KSPROPERTY_DESCRIPTION);
        }
    } 
    else if(PropertyRequest->ValueSize >= sizeof(ULONG))
    {
        // if return buffer can hold a ULONG, return the access flags
        PULONG AccessFlags = PULONG(PropertyRequest->Value);

        PropertyRequest->ValueSize = sizeof(ULONG);
        *AccessFlags = KSPROPERTY_TYPE_ALL;
    }
    else
    {
        PropertyRequest->ValueSize = 0;
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    return ntStatus;
}

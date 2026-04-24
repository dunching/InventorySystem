#include "ItemProxy_Modifier.h"
#include "ItemSystemTags.h"

bool FItemProxy_Modifier::IsInUse() const
{
	return UsingEquipmentProxyId.IsValid();
}

bool FItemProxy_Modifier::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	if (!FBasicProxy::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	Ar << UsingEquipmentProxyId;
	bOutSuccess = true;
	return true;
}

void FItemProxy_Modifier::SetUsingEquipmentProxyId(const FGuid& InEquipmentProxyId)
{
	UsingEquipmentProxyId = InEquipmentProxyId;
}

void FItemProxy_Modifier::ClearUsingEquipmentProxyId()
{
	UsingEquipmentProxyId.Invalidate();
}

const FGuid& FItemProxy_Modifier::GetUsingEquipmentProxyId() const
{
	return UsingEquipmentProxyId;
}

FGameplayTag FProxy_ModifierStrategy::GetProxySchemaTag() const
{
	return ItemSystemTags::Inventory_ProxySchema_Modifier;
}

TSharedPtr<FBasicProxy> FProxy_ModifierStrategy::GetProxy() const
{
	return MakeShared<FItemProxy_Modifier>();
}

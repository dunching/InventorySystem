#include "ItemProxy_Equipment.h"
#include "ItemSystemTags.h"

bool FItemProxy_Equipment::IsEquipmentProxy() const
{
	return true;
}

bool FItemProxy_Equipment::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	if (!FBasicProxy::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	int32 ModifierCount = InstalledModifierProxyIds.Num();
	Ar << ModifierCount;

	if (Ar.IsLoading())
	{
		InstalledModifierProxyIds.Empty(ModifierCount);
		for (int32 Index = 0; Index < ModifierCount; ++Index)
		{
			FGuid ModifierProxyId;
			Ar << ModifierProxyId;
			InstalledModifierProxyIds.Add(ModifierProxyId);
		}
	}
	else
	{
		for (FGuid ModifierProxyId : InstalledModifierProxyIds)
		{
			Ar << ModifierProxyId;
		}
	}

	bOutSuccess = true;
	return true;
}

bool FItemProxy_Equipment::AddInstalledModifierProxyId(const FGuid& ModifierProxyId)
{
	if (!ModifierProxyId.IsValid() || InstalledModifierProxyIds.Contains(ModifierProxyId))
	{
		return false;
	}

	InstalledModifierProxyIds.Add(ModifierProxyId);
	return true;
}

bool FItemProxy_Equipment::RemoveInstalledModifierProxyId(const FGuid& ModifierProxyId)
{
	return InstalledModifierProxyIds.RemoveSingle(ModifierProxyId) > 0;
}

bool FItemProxy_Equipment::HasInstalledModifierProxyId(const FGuid& ModifierProxyId) const
{
	return InstalledModifierProxyIds.Contains(ModifierProxyId);
}

void FItemProxy_Equipment::SetInstalledModifierProxyIds(const TArray<FGuid>& InModifierProxyIds)
{
	InstalledModifierProxyIds = InModifierProxyIds;
}

const TArray<FGuid>& FItemProxy_Equipment::GetInstalledModifierProxyIds() const
{
	return InstalledModifierProxyIds;
}

FGameplayTag FProxy_EquipmentStrategy::GetProxySchemaTag() const
{
	return ItemSystemTags::Inventory_ProxySchema_Equipment;
}

TSharedPtr<FBasicProxy> FProxy_EquipmentStrategy::GetProxy() const
{
	return MakeShared<FItemProxy_Equipment>();
}

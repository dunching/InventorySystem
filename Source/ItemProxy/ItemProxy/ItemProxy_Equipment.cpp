#include "ItemProxy_Equipment.h"
#include "ItemSystemTags.h"

bool FItemProxy_Equipment::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	// 装备 Proxy 目前沿用基础序列化逻辑。
	return FBasicProxy::NetSerialize(Ar, Map, bOutSuccess);
}

FGameplayTag FProxy_EquipmentStrategy::GetProxySchemaTag() const
{
	return ItemSystemTags::Inventory_ProxySchema_Equipment;
}

TSharedPtr<FBasicProxy> FProxy_EquipmentStrategy::GetProxy() const
{
	return MakeShared<FItemProxy_Equipment>();
}

#include "../otpch.h"

#include "enums.h"

std::string_view tfs::http::detail::getPasswordStrengthColor(int strength)
{
	switch (strength) {
		case 0:
		case 1:
			return "#EC644B";
		case 2:
			return "#EB8005";
		case 3:
			return "#B0B300";
		case 4:
			return "#20A000";
	}

	std::unreachable();
}

int tfs::http::detail::getPvpTypeIndex(WorldType_t worldType)
{
	switch (worldType) {
		case WORLD_TYPE_PVP:
			return 0;
		case WORLD_TYPE_NO_PVP:
			return 1;
		case WORLD_TYPE_PVP_ENFORCED:
			return 2;
	}

	std::unreachable();
}

std::string_view tfs::http::detail::getPvpTypeName(WorldType_t worldType)
{
	switch (worldType) {
		case WORLD_TYPE_PVP:
			return "Open PvP";
		case WORLD_TYPE_NO_PVP:
			return "Optional PvP";
		case WORLD_TYPE_PVP_ENFORCED:
			return "Hardcore PvP";
	}

	std::unreachable();
}
#include "stdafx.h"

#define AUTHCACHE_FILEPATH "players/auth.bin"

namespace LiveSteam
{
	class AuthCache
	{
	private:
		char* m_steamAppTicket;
		char* m_steamCookieKey;

		unsigned int m_steamAppTicketSize;
		static const unsigned int m_steamCookieKeySize = sizeof(g_authService->m_steamCookieKey);

	public:
		AuthCache(void);
		~AuthCache(void);

		// Check if the current cache is valid
		bool ValidateCache(void) const;

		// Reload the cache from the disk
		int ReloadCache(void);

		// Update the cache values from user defined values
		int UpdateCache(void* appTicket, unsigned int appTicketSize, char* cookieKey);

		// Commit the cache to the disk
		int CommitCache(void) const;

		// Apply the cache contents to given addresses
		int ApplyCache(void* ticketBuf, unsigned int ticketBufSize, unsigned int* ticketSize, char* cookieKey);

		// Invalidate the cache & free any allocated buffers
		void ClearCache(void);
	};
}

static LiveSteam::AuthCache g_authCache;

EResult LiveSteamClient::rawTicketResult;

namespace LiveSteam
{
	AuthCache::AuthCache(void) : m_steamAppTicket(nullptr), m_steamAppTicketSize(0), m_steamCookieKey(nullptr)
	{
		ReloadCache();
	}

	AuthCache::~AuthCache(void)
	{
		ClearCache();
	}

	bool AuthCache::ValidateCache(void) const
	{
		return m_steamAppTicket != nullptr
			&& m_steamCookieKey != nullptr
			&& m_steamAppTicketSize != 0;
	}

	int AuthCache::ReloadCache(void)
	{
		FILE *h = nullptr;
		if (fopen_s(&h, AUTHCACHE_FILEPATH, "rb") != 0)
		{
			// auth cache file could not be loaded
			ClearCache();
			return 1;
		}

		//
		// Error handler lambda that automatically:
		//  + frees the cache file handle
		//  + deletes the cache file
		//  + clears the memory cache
		//
		auto DeleteBadCache = [this, h](void)
		{
			// Close the file handle so we can actually delete the file
			fclose(h);

			DeleteFileA(AUTHCACHE_FILEPATH);
			ClearCache();
		};

		//
		// Check the age of the cache file
		// If it's too old we need to delete it and clear the cache
		//
		if (FS_FileAge_Sec(AUTHCACHE_FILEPATH) > 60)
		{
			DeleteBadCache();
			return 5;
		}

		// We only need to allocate a buffer for m_steamCookieKey if it doesn't already have one.
		// Since m_steamCookieKeySize is constant, we don't need to worry about resizing the buffer
		if (m_steamCookieKey == nullptr)
			m_steamCookieKey = new char[m_steamCookieKeySize];

		if (fread(m_steamCookieKey, 1, m_steamCookieKeySize, h) != m_steamCookieKeySize)
		{
			// Unable to read the correct number of bytes for the cookie key
			DeleteBadCache();
			return 2;
		}

		// All of the remaining data is the appTicket, so let's figure out how big the ticket is
		// by determining the size of the remaining data
		unsigned int cur = ftell(h);
		fseek(h, 0, SEEK_END);
		unsigned int end = ftell(h);
		fseek(h, cur, SEEK_SET);

		unsigned int oldTicketSize = m_steamAppTicketSize;
		m_steamAppTicketSize = end - cur;

		// Only bother reallocating the appTicket buffer if the old ticket was smaller than the new one
		// or if the buffer wasn't allocated yet
		if (m_steamAppTicket == nullptr || oldTicketSize < m_steamAppTicketSize)
		{
			delete[] m_steamAppTicket;
			m_steamAppTicket = new char[m_steamAppTicketSize];
		}

		if (fread(m_steamAppTicket, 1, m_steamAppTicketSize, h) != m_steamAppTicketSize)
		{
			// Unable to read the correct number of bytes for the app ticket
			DeleteBadCache();
			return 3;
		}

		fclose(h);

		return ValidateCache() ? 0 : 4;
	}

	int AuthCache::UpdateCache(void* appTicket, unsigned int appTicketSize, char* cookieKey)
	{
		m_steamAppTicketSize = appTicketSize;

		delete[] m_steamAppTicket;
		m_steamAppTicket = new char[m_steamAppTicketSize];
		memcpy(m_steamAppTicket, appTicket, m_steamAppTicketSize);

		delete[] m_steamCookieKey;
		m_steamCookieKey = new char[m_steamCookieKeySize];

		memcpy(m_steamCookieKey, cookieKey, m_steamCookieKeySize);

		return 0;
	}

	int AuthCache::CommitCache(void) const
	{
		ASSERT(ValidateCache());

		FILE *h = nullptr;

		if (fopen_s(&h, AUTHCACHE_FILEPATH, "wb") != 0)
		{
			// auth cache file could not be written
			return 1;
		}

		fwrite(m_steamCookieKey, 1, m_steamCookieKeySize, h);
		fwrite(m_steamAppTicket, 1, m_steamAppTicketSize, h);

		fclose(h);

		return 0;
	}

	int AuthCache::ApplyCache(void* ticketBuf, unsigned int ticketBufSize, unsigned int* ticketSize, char* cookieKey)
	{
		if (int err = ReloadCache())
			return err;

		ASSERT(m_steamAppTicket != nullptr);
		ASSERT(m_steamCookieKey != nullptr);

		ASSERT(m_steamAppTicketSize <= ticketBufSize);

		*ticketSize = m_steamAppTicketSize;

		memcpy(ticketBuf, m_steamAppTicket, min(m_steamAppTicketSize, ticketBufSize));
		memcpy(cookieKey, m_steamCookieKey, m_steamCookieKeySize);
		
		return 0;
	}

	void AuthCache::ClearCache(void)
	{
		m_steamAppTicketSize = 0;
		delete[] m_steamAppTicket;
		delete[] m_steamCookieKey;
		m_steamAppTicket = nullptr;
		m_steamCookieKey = nullptr;
	}
}

// /live/live_steam_client.cpp:18
void LiveSteamClient::RequestEncryptedAppTicket(const void *data, const unsigned int size)
{
	ASSERT_MSG(false, "Function not implemented");
}

// /live/live_steam_client.cpp:25
bool LiveSteamClient::GetRetrievedEncryptedAppTicket(void *ticketBuf, const unsigned int ticketBufSize, unsigned int *ticketSize)
{
	ASSERT(resultOnRequestEncryptedAppTicket == k_EResultOK && ticketBufSize);

	// SteamUser()->GetEncryptedAppTicket(void *pTicket, int cbMaxTicket, uint32 *pcbTicket);
	auto steamUser = (*(void *(__cdecl **)())0x009A356C)();
	auto getEncryptedAppTicket = *(bool(__thiscall **)(void *, void *, int, unsigned int *))(*(DWORD *)steamUser + 80);

	bool useCache = dw_cacheTicket && dw_cacheTicket->current.enabled;

	//
	// Use the real function if one of the following conditions are met:
	//	+ Steam had a good ticket
	//	+ The caching subsystem is disabled
	//
	// Otherwise we fallback to using the cached ticket
	//
	if (this->rawTicketResult == k_EResultOK || !useCache)
	{
		if (getEncryptedAppTicket(steamUser, ticketBuf, ticketBufSize, ticketSize))
		{
			if (useCache)
			{
				DBG_ASSERT(g_authService);
				if (!g_authService)
				{
					Com_DPrintf(1, "STEAM: Couldn't cache recieved auth ticket - g_authService is NULL\n");
					return true;
				}

				// Update the cache with the generated ticket & DW cookie
				g_authCache.UpdateCache(ticketBuf, *ticketSize, g_authService->m_steamCookieKey);

				// Commit the new cache contents to the disk
				if (g_authCache.CommitCache() == 0)
				{
					Com_DPrintf(1, "STEAM: Wrote cached auth ticket\n");
				}
			}

			Com_DPrintf(1, "STEAM: Retrieved auth ticket from Steam, sending to DemonWare\n");
			return true;
		}
	}
	else
	{
		DBG_ASSERT(g_authService);
		if (!g_authService)
		{
			Com_DPrintf(1, "STEAM: Couldn't use cached auth ticket - g_authService is NULL\n");
			return false;
		}

		int err = g_authCache.ApplyCache(ticketBuf, ticketBufSize, ticketSize, g_authService->m_steamCookieKey);
		if (err == 0)
		{
			Com_DPrintf(1, "STEAM: Retrieved cached auth ticket, sending to DemonWare\n");
			return true;
		}

		Com_DPrintf(1, "^1STEAM: Load cache - error %d\n", err);
		g_authCache.ClearCache();
	}

	Com_DPrintf(1, "STEAM: Failed to retrieve ticket\n");
	return false;
}

// /live/live_steam_client.cpp:33
void LiveSteamClient::OnRequestEncryptedAppTicket(EncryptedAppTicketResponse_t *pEncryptedAppTicketResponse, bool bIOFailure)
{
	if (bIOFailure)
		return;

	this->rawTicketResult = pEncryptedAppTicketResponse->m_eResult;

	switch (pEncryptedAppTicketResponse->m_eResult)
	{
	case k_EResultOK:
		dwLogonSeAcquiredSteamTicket();
		break;
	case k_EResultLimitExceeded:
		Com_DPrintf(1, "STEAM: Calling RequestEncryptedAppTicket more than once per minute returns this error\n");
		break;
	case k_EResultDuplicateRequest:
		Com_DPrintf(1, "STEAM: Calling RequestEncryptedAppTicket while there is already a pending request results in this error\n");
		break;
	case k_EResultNoConnection:
		Com_DPrintf(1, "STEAM: Calling RequestEncryptedAppTicket while not connected to steam results in this error\n");
		break;
	}

	//
	// Only use the caching subsystem if we explicitly have it enabled
	//
	if (dw_cacheTicket && dw_cacheTicket->current.enabled)
	{
		// If the request was rate limited, check if there's a cached ticket
		if (pEncryptedAppTicketResponse->m_eResult == k_EResultLimitExceeded)
		{
			if (g_authCache.ValidateCache())
			{
				Com_DPrintf(1, "STEAM: Attempting to use cached auth ticket...\n");

				pEncryptedAppTicketResponse->m_eResult = k_EResultOK;
				dwLogonSeAcquiredSteamTicket();
			}
		}
	}

	this->resultOnRequestEncryptedAppTicket = pEncryptedAppTicketResponse->m_eResult;
}

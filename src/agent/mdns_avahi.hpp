/*
 *    Copyright (c) 2017, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *    POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file includes definition for MDNS service based on avahi.
 */

#ifndef MDNS_AVAHI_HPP_
#define MDNS_AVAHI_HPP_

#include <vector>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/domain.h>
#include <avahi-common/watch.h>
#include <sys/select.h>

#include "mdns.hpp"

/**
 * @addtogroup border-router-mdns
 *
 * @brief
 *   This module includes definition for avahi-based MDNS service.
 *
 * @{
 */

/**
 * This structure implements AvahiWatch.
 *
 */
struct AvahiWatch
{
    int                mFd;       ///< The file descriptor to watch.
    AvahiWatchEvent    mEvents;   ///< The interested events.
    int                mHappened; ///< The events happened.
    AvahiWatchCallback mCallback; ///< The function to be called when interested events happened on mFd.
    void *             mContext;  ///< A pointer to application-specific context.
    void *             mPoller;   ///< The poller created this watch.

    /**
     * The constructor to initialize an Avahi watch.
     *
     * @param[in]   aFd         The file descriptor to watch.
     * @param[in]   aEvents     The events to watch.
     * @param[in]   aCallback   The function to be called when events happend on this file descriptor.
     * @param[in]   aContext    A pointer to application-specific context.
     * @param[in]   aPoller     The Poller this watcher belongs to.
     *
     */
    AvahiWatch(int aFd, AvahiWatchEvent aEvents, AvahiWatchCallback aCallback, void *aContext, void *aPoller)
        : mFd(aFd)
        , mEvents(aEvents)
        , mCallback(aCallback)
        , mContext(aContext)
        , mPoller(aPoller)
    {
    }
};

/**
 * This structure implements the AvahiTimeout.
 *
 */
struct AvahiTimeout
{
    unsigned long        mTimeout;  ///< Absolute time when this timer timeout.
    AvahiTimeoutCallback mCallback; ///< The function to be called when timeout.
    void *               mContext;  ///< The pointer to application-specific context.
    void *               mPoller;   ///< The poller created this timer.

    /**
     * The constructor to initialize an AvahiTimeout.
     *
     * @param[in]   aTimeout    A pointer to the time after which the callback should be called.
     * @param[in]   aCallback   The function to be called after timeout.
     * @param[in]   aContext    A pointer to application-specific context.
     * @param[in]   aPoller     The Poller this timeout belongs to.
     *
     */
    AvahiTimeout(const struct timeval *aTimeout, AvahiTimeoutCallback aCallback, void *aContext, void *aPoller);
};

namespace ot {

namespace BorderRouter {

namespace Mdns {

/**
 * This class implements the AvahiPoll.
 *
 */
class Poller
{
public:
    /**
     * The constructor to initialize a Poller.
     *
     */
    Poller(void);

    /**
     * This method updates the fd_set and timeout for mainloop.
     *
     * @param[inout]    aReadFdSet      A reference to fd_set for polling read.
     * @param[inout]    aWriteFdSet     A reference to fd_set for polling write.
     * @param[inout]    aErrorFdSet     A reference to fd_set for polling error.
     * @param[inout]    aMaxFd          A reference to the max file descriptor.
     * @param[inout]    aTimeout        A reference to the timeout.
     *
     */
    void UpdateFdSet(fd_set &aReadFdSet, fd_set &aWriteFdSet, fd_set &aErrorFdSet, int &aMaxFd, timeval &aTimeout);

    /**
     * This method performs avahi poll processing.
     *
     * @param[in]   aReadFdSet   A reference to read file descriptors.
     * @param[in]   aWriteFdSet  A reference to write file descriptors.
     * @param[in]   aErrorFdSet  A reference to error file descriptors.
     *
     */
    void Process(const fd_set &aReadFdSet, const fd_set &aWriteFdSet, const fd_set &aErrorFdSet);

    /**
     * This method returns the AvahiPoll.
     *
     * @returns A pointer to the AvahiPoll.
     *
     */
    const AvahiPoll *GetAvahiPoll(void) const { return &mAvahiPoller; }

private:
    typedef std::vector<AvahiWatch *>   Watches;
    typedef std::vector<AvahiTimeout *> Timers;

    static AvahiWatch *    WatchNew(const struct AvahiPoll *aPoller,
                                    int                     aFd,
                                    AvahiWatchEvent         aEvent,
                                    AvahiWatchCallback      aCallback,
                                    void *                  aContext);
    AvahiWatch *           WatchNew(int aFd, AvahiWatchEvent aEvent, AvahiWatchCallback aCallback, void *aContext);
    static void            WatchUpdate(AvahiWatch *aWatch, AvahiWatchEvent aEvent);
    static AvahiWatchEvent WatchGetEvents(AvahiWatch *aWatch);
    static void            WatchFree(AvahiWatch *aWatch);
    void                   WatchFree(AvahiWatch &aWatch);
    static AvahiTimeout *  TimeoutNew(const AvahiPoll *     aPoller,
                                      const struct timeval *aTimeout,
                                      AvahiTimeoutCallback  aCallback,
                                      void *                aContext);
    AvahiTimeout *         TimeoutNew(const struct timeval *aTimeout, AvahiTimeoutCallback aCallback, void *aContext);
    static void            TimeoutUpdate(AvahiTimeout *aTimer, const struct timeval *aTimeout);
    static void            TimeoutFree(AvahiTimeout *aTimer);
    void                   TimeoutFree(AvahiTimeout &aTimer);

    Watches   mWatches;
    Timers    mTimers;
    AvahiPoll mAvahiPoller;
};

/**
 * This class implements MDNS service with avahi.
 *
 */
class PublisherAvahi : public Publisher
{
public:
    /**
     * The constructor to initialize a Publisher.
     *
     * @param[in]   aProtocol           The protocol used for publishing. IPv4, IPv6 or both.
     * @param[in]   aHost               The name of host residing the services to be published.
                                        NULL to use default.
     * @param[in]   aDomain             The domain of the host. NULL to use default.
     * @param[in]   aHandler            The function to be called when state changes.
     * @param[in]   aContext            A pointer to application-specific context.
     *
     */
    PublisherAvahi(int aProtocol, const char *aHost, const char *aDomain, StateHandler aHandler, void *aContext);

    ~PublisherAvahi(void);

    /**
     * This method publishes or updates a service.
     *
     * @note only text record can be updated.
     *
     * @param[in]   aName               The name of this service.
     * @param[in]   aType               The type of this service.
     * @param[in]   aPort               The port number of this service.
     * @param[in]   ...                 Pointers to null-terminated string of key and value for text record.
     *                                  The last argument must be NULL.
     *
     * @retval  OTBR_ERROR_NONE     Successfully published or updated the service.
     * @retval  OTBR_ERROR_ERRNO    Failed to publish or update the service.
     *
     */
    otbrError PublishService(uint16_t aPort, const char *aName, const char *aType, ...);

    /**
     * This method starts the MDNS service.
     *
     * @retval OTBR_ERROR_NONE  Successfully started MDNS service;
     * @retval OTBR_ERROR_MDNS  Failed to start MDNS service.
     *
     */
    otbrError Start(void);

    /**
     * This method checks if publisher has been started.
     *
     * @retval true     Already started.
     * @retval false    Not started.
     *
     */
    bool IsStarted(void) const;

    /**
     * This method stops the MDNS service.
     *
     */
    void Stop(void);

    /**
     * This method performs avahi poll processing.
     *
     * @param[in]   aReadFdSet          A reference to read file descriptors.
     * @param[in]   aWriteFdSet         A reference to write file descriptors.
     * @param[in]   aErrorFdSet         A reference to error file descriptors.
     *
     */
    void Process(const fd_set &aReadFdSet, const fd_set &aWriteFdSet, const fd_set &aErrorFdSet);

    /**
     * This method updates the fd_set and timeout for mainloop.
     *
     * @param[inout]    aReadFdSet      A reference to fd_set for polling read.
     * @param[inout]    aWriteFdSet     A reference to fd_set for polling write.
     * @param[inout]    aErrorFdSet     A reference to fd_set for polling error.
     * @param[inout]    aMaxFd          A reference to the max file descriptor.
     * @param[inout]    aTimeout        A reference to the timeout.
     *
     */
    void UpdateFdSet(fd_set &aReadFdSet, fd_set &aWriteFdSet, fd_set &aErrorFdSet, int &aMaxFd, timeval &aTimeout);

private:
    enum
    {
        kMaxSizeOfTxtRecord   = 128,
        kMaxSizeOfServiceName = AVAHI_LABEL_MAX,
        kMaxSizeOfHost        = AVAHI_LABEL_MAX,
        kMaxSizeOfDomain      = AVAHI_LABEL_MAX,
        kMaxSizeOfServiceType = AVAHI_LABEL_MAX,
    };

    struct Service
    {
        char     mName[kMaxSizeOfServiceName];
        char     mType[kMaxSizeOfServiceType];
        uint16_t mPort;
    };

    typedef std::vector<Service> Services;

    static void HandleClientState(AvahiClient *aClient, AvahiClientState aState, void *aContext);
    void        HandleClientState(AvahiClient *aClient, AvahiClientState aState);

    void        CreateGroup(AvahiClient *aClient);
    static void HandleGroupState(AvahiEntryGroup *aGroup, AvahiEntryGroupState aState, void *aContext);
    void        HandleGroupState(AvahiEntryGroup *aGroup, AvahiEntryGroupState aState);

    Services         mServices;
    AvahiClient *    mClient;
    AvahiEntryGroup *mGroup;
    Poller           mPoller;
    int              mProtocol;
    char *           mHost;
    char *           mDomain;
    State            mState;
    StateHandler     mStateHandler;
    void *           mContext;
};

} // namespace Mdns

} // namespace BorderRouter

} // namespace ot

/**
 * @}
 */
#endif // MDNS_AVAHI_HPP_

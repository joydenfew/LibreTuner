/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBRETUNER_VEHICLE_H
#define LIBRETUNER_VEHICLE_H

#include <memory>
#include <string>


class Definition;
using DefinitionPtr = std::shared_ptr<Definition>;

class DataLogger;
using DataLoggerPtr = std::shared_ptr<DataLogger>;

class DataLink;
using DataLinkPtr = std::shared_ptr<DataLink>;

class DownloadInterface;
using DownloadInterfacePtr = std::shared_ptr<DownloadInterface>;

class DiagnosticsInterface;

class Flasher;
using FlasherPtr = std::shared_ptr<Flasher>;

namespace isotp {
class Protocol;
}

namespace uds {
class Protocol;
}

class CanInterface;
using CanInterfacePtr = std::shared_ptr<CanInterface>;

struct Vehicle {
    std::string name;
    std::string vin;
    DefinitionPtr definition;

    bool valid() const { return !vin.empty(); }

    static Vehicle fromVin(const std::string &vin);
};

// VehicleLink is DataLink + vehicle-specific optons (like the CAN bus baudrate)
class VehicleLink {
public:
    VehicleLink(const Vehicle &vehicle, const DataLinkPtr &link)
        : vehicle_(vehicle), datalink_(link) {}
    VehicleLink(Vehicle &&vehicle, const DataLinkPtr &link)
        : vehicle_(std::move(vehicle)), datalink_(link) {}

    ~VehicleLink();

    /* Returns a logger suitable for logging from the vehicle using the
       datalink. Returns nullptr if a logger could not be created. */
    std::unique_ptr<DataLogger> logger() const;

    /* Returns a usable download interface for the link, if one exists. May
     * return nullptr. */
    std::unique_ptr<DownloadInterface> downloader() const;

    /* Returns a diagnostic interface for the link. If none exist, returns
     * nullptr. */
    std::unique_ptr<DiagnosticsInterface> diagnostics() const;

    /* Returns a flash interface for flashing, if one exists. May return
     * nullptr. */
    std::unique_ptr<Flasher> flasher() const;

    /* Returns a UDS interface. May return nullptr if the vehicle/datalink
     * does not support it */
    std::unique_ptr<uds::Protocol> uds() const;

    /* Returns an ISO-TP interface. May return nullptr if the vehicle/datalink
     * does not support it */
    std::unique_ptr<isotp::Protocol> isotp() const;

    /* Returns a CAN interface. May return nullptr if the vehicle/data
     * does not support CAN */
    std::unique_ptr<CanInterface> can() const;

    const Vehicle &vehicle() const { return vehicle_; }

private:
    Vehicle vehicle_;
    DataLinkPtr datalink_;
};

#endif // LIBRETUNER_VEHICLE_H

#ifndef LT_ISOTPELM_H
#define LT_ISOTPELM_H

#include "isotp.h"
#include "../command/elm327.h"

namespace lt::network {

class IsoTpElm : public IsoTp {
public:
    // Constructs from an opened Elm327 device
    explicit IsoTpElm(Elm327Ptr device, IsoTpOptions options = IsoTpOptions());

    void recv(IsoTpPacket &result) override;

    void request(const IsoTpPacket &req, IsoTpPacket &result) override;

    void send(const IsoTpPacket &packet) override;

    void setOptions(const IsoTpOptions &options) override;

    // Updates header and receive ids
    void updateOptions();

private:
    Elm327Ptr device_;
    IsoTpOptions options_;
};

}


#endif //LIBRETUNER_ISOTPELM_H
#include <iostream>
#include <fingera/ocl/device.hpp>
#include <boost/compute/device.hpp>
#include <boost/compute/platform.hpp>
#include <boost/compute/system.hpp>

using bcc = boost::compute::device;
using bcs = boost::compute::system;

namespace fingera {
namespace ocl {

static void dump_device(const bcc &dev, const std::string &prefix = "") {
    std::cout << prefix << "name: " << dev.name() << std::endl;
    std::cout << prefix << "type: ";
    switch (dev.type()) {
        case bcc::type::cpu:
            std::cout << prefix << "cpu";
            break;
        case bcc::type::gpu:
            std::cout << prefix << "gpu";
            break;
        case bcc::type::accelerator:
            std::cout << prefix << "accelerator";
            break;
        default:
            std::cout << prefix << "unknown";
            break;
    }
    std::cout << prefix << std::endl;
    std::cout << prefix << "vendor: " << dev.vendor() << std::endl;
    std::cout << prefix << "profile: " << dev.profile() << std::endl;
    std::cout << prefix << "version: " << dev.version() << std::endl;
    std::cout << prefix << "driver_version: " << dev.driver_version() << std::endl;
    std::cout << prefix << "extensions: " << dev.get_info<std::string>(CL_DEVICE_EXTENSIONS) << std::endl;
    std::cout << prefix << "address_bits: " << dev.address_bits() << std::endl;
    std::cout << prefix << "global_memory_size: " << dev.global_memory_size() << std::endl;
    std::cout << prefix << "local_memory_size: " << dev.local_memory_size() << std::endl;
    std::cout << prefix << "clock_frequency: " << dev.clock_frequency() << std::endl;
    std::cout << prefix << "compute_units: " << dev.compute_units() << std::endl;
    std::cout << prefix << "max_memory_alloc_size: " << dev.max_memory_alloc_size() << std::endl;
    std::cout << prefix << "max_work_group_size: " << dev.max_work_group_size() << std::endl;
    std::cout << prefix << "max_work_item_dimensions: " << dev.max_work_item_dimensions() << std::endl;
    std::cout << prefix << "profiling_timer_resolution: " << dev.profiling_timer_resolution() << std::endl;
    std::cout << prefix << "is_subdevice: " << (dev.is_subdevice() ? "true" : "false") << std::endl;
    std::cout << prefix << std::endl;
}


void dump_platforms() {
    auto platforms = bcs::platforms();
    std::cout << "------------------- OCL Platforms -------------------" << std::endl;
    for (auto &pfm : platforms) {
        std::cout << "name: " << pfm.name() << std::endl;
        std::cout << "vendor: " << pfm.vendor() << std::endl;
        std::cout << "profile: " << pfm.profile() << std::endl;
        std::cout << "version: " << pfm.version() << std::endl;
        std::cout << "extensions: " << pfm.get_info<std::string>(CL_PLATFORM_EXTENSIONS) << std::endl;
        std::cout << "devices: " << std::endl;
        auto devs = pfm.devices();
        for (auto &dev : devs) {
            dump_device(dev, "\t");
        }
    }
}

void dump_devices() {
    auto devices = bcs::devices();
    std::cout << "------------------- OCL Devices -------------------" << std::endl;
    for (auto &dev : devices) {
        dump_device(dev);
    }
}

} // namespace ocl
} // namespace fingera
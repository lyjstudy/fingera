#include <iostream>
#include <fingera/ocl/device.hpp>
#include <fingera/hex.hpp>
#include <boost/compute/buffer.hpp>
#include <boost/compute/platform.hpp>
#include <boost/compute/system.hpp>

namespace bc = boost::compute;
using bcc = bc::device;
using bcs = bc::system;

const char *sha256CL =
        #include "./sha256.cl"
;

namespace fingera {
namespace ocl {

void test_device() {
    auto device = bcs::default_device();
    bc::context context(device);
    bc::command_queue queue(context, device);

    auto program = bc::program::create_with_source(sha256CL, context);
    program.build();
    bc::kernel kernel(program, "sha256_process_trunk");

    bc::buffer chunk(context, 64, bc::memory_object::mem_flags::read_only);
    bc::buffer digest(context, 32 * 100, bc::memory_object::mem_flags::write_only);
    kernel.set_arg(0, chunk);
    kernel.set_arg(1, digest);

    uint8_t sha256_single_block[] = {
        // data
        0x30,
        // pad
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // length
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
    };
    size_t global_work_size = 64;
    size_t local_work_size = 1;
    queue.enqueue_write_buffer(chunk, 0, sizeof(sha256_single_block), sha256_single_block);
    queue.enqueue_1d_range_kernel(kernel, 0, global_work_size, local_work_size);
    queue.finish();
    uint8_t data[32 * 100];
    memset(data, 0, sizeof(data));
    queue.enqueue_read_buffer(digest, 0, sizeof(data), data);
    for (size_t i = 0; i < 66; i++) {
        std::cout << fingera::to_hex(data + i * 32, 32) << std::endl;
    }
}

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
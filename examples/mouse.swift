/**
 * Create a virtual mouse.
 * To run:  swift mouse.swift
 * Stop with CTRL-C.
 * Install kernel extension from https://github.com/unbit/foohid/releases/latest first.
 */

import IOKit

let report_descriptor: [CUnsignedChar] = [
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,                    // USAGE (Mouse)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x09, 0x01,                    //   USAGE (Pointer)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x03,                    //     REPORT_COUNT (3)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x75, 0x05,                    //     REPORT_SIZE (5)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
	0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x02,                    //     REPORT_COUNT (2)
	0x81, 0x06,                    //     INPUT (Data,Var,Rel)
	0xc0,                          //   END_COLLECTION
	0xc0                           // END_COLLECTION
]

struct mouse_report_t {
	let buttons: UInt8
	let x: Int8
	let y: Int8
}

let SERVICE_NAME = "it_unbit_foohid"
let FOOHID_CREATE: UInt32 = 0 // create selector
let FOOHID_SEND:   UInt32 = 2 // send selector
let DEVICE_NAME =  "Foohid Virtual Mouse"
let DEVICE_SN =    "SN 123456"

enum VirtualInputError: Error, CustomStringConvertible {
	case notAccessIOService
	case notOpenIOService
	case notSendMessageToHID

	var description: String {
		switch self {
		case .notAccessIOService:
			return "Unable to access IOService."
		case .notOpenIOService:
			return "Unable to open IOService."
		case .notSendMessageToHID:
			return "Unable to send message to HID device."
		}
	}
}

func main() throws {
	var iterator: io_iterator_t = 0
	var connect: io_connect_t = 0

	// Get a reference to the IOService
	guard IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(SERVICE_NAME), &iterator) == KERN_SUCCESS
		else { throw VirtualInputError.notAccessIOService }
	defer { IOObjectRelease(iterator) }

	guard let service: io_object_t = (sequence(state: iterator) {
		let service = IOIteratorNext($0)
		return service != IO_OBJECT_NULL ? service : nil
		}.first {
			if IOServiceOpen($0, mach_task_self_, 0, &connect) == KERN_SUCCESS {
				return true
			}
			IOObjectRelease($0)
			return false
	}) else { throw VirtualInputError.notOpenIOService }
	defer { IOObjectRelease(service) }

	let DEVICE_NAME_POINTER = unsafeBitCast(strdup(DEVICE_NAME), to: UInt64.self)
	let DEVICE_NAME_LENGTH = UInt64(DEVICE_NAME.utf8.count)

	report_descriptor.withUnsafeBufferPointer { report_descriptor in
		// Fill up the input arguments.
		let input_count: UInt32 = 8
		var input = [UInt64](repeatElement(0, count: Int(input_count)))
		input[0] = DEVICE_NAME_POINTER	// device name
		input[1] = DEVICE_NAME_LENGTH // name length
		input[2] = unsafeBitCast(report_descriptor.baseAddress, to: UInt64.self) // report descriptor
		input[3] = UInt64(report_descriptor.count) // report descriptor len
		input[4] = unsafeBitCast(strdup(DEVICE_SN), to: UInt64.self) // serial number
		input[5] = UInt64(DEVICE_SN.utf8.count) // serial number len
		input[6] = 2 // vendor ID
		input[7] = 3 // device ID

		if IOConnectCallScalarMethod(connect, FOOHID_CREATE, input, input_count, nil, nil) != KERN_SUCCESS {
			print("Unable to create HID device. May be fine if created previously.")
		}
	}

	// Arguments to be passed through the HID message.
	let send_count: UInt32 = 4
	var send = [UInt64](repeatElement(0, count: Int(send_count)))
	send[0] = DEVICE_NAME_POINTER // device name
	send[1] = DEVICE_NAME_LENGTH // name length

	send[3] = UInt64(MemoryLayout<mouse_report_t>.stride) // mouse struct len

	while true {
		var mouse = mouse_report_t(buttons: 0, x: 20, y: 20)
		try withUnsafePointer(to: &mouse) { mouse in
			send[2] = UInt64(UInt(bitPattern: mouse)) // mouse struct
			guard IOConnectCallScalarMethod(connect, FOOHID_SEND, send, send_count, nil, nil) == KERN_SUCCESS
				else { throw VirtualInputError.notSendMessageToHID }
		}
		sleep(1)
	}
}

do {
	try main()
} catch {
	print(error)
	exit(1)
}


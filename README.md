# CAN to MQTT
This repository implements a CAN message to MQTT message function.
The CAN message is first decoded into DBC signal values.
TheDBC signal values are published to a remote MQTT broker.

This repository consists of a library, a Google Unit Test executable 
and an end-user app.
The library contains all main features and can be included in other 
applications as the Bus Master GUI app.

The repository is a prototype.
The following features are not yet defined.
- TLS support
- When to send changed CAN signals to the MQTT broker.

## The CAN Bus
The CAN messages are received from an internal shared memory bus. 
This bus is defined in the Bus Message repository.
The end-user must define, which messages that should be handled by this library.

## The DBC Parsing
The CAN message is parsed into DBC signal values.
This is mainly done by the DBC repository.

## The MQTT Interface
The signals are converted to scaled values, the last reported value 
and its timestamp is stored in a metric database.
This database is in turn used by the MQTT library and is in the Metric repository.

The MQTT interface is implemented by using the new Boost MQTT 5 interface.
The MQTT metric is implemented in the MQTT Metric repository.

## The CAN to MQTT App
The app should be started with an input config file. 
The config file defines the MQTT broker host and port, the DBC file and,
which messages and signals that should be sent to the broker.

# graphite-csv-generator

Graphite is a database for numeric time-series data.
It's HTTP API allows us to render certain metrics in different formats like JSON, PNG or CSV. See [Graphite HTTP API.](https://graphite-api.readthedocs.io/en/latest/api.html)

## Problem
When rendering metrics into the CSV format, all data will be written **into a single column**. Obviously, this is very unpractical. The API call would be
```
http://<GRAPHITE_IP>/render?target=<METRICS>&format=csv
```
Example call:
```http://192.168.8.42:81/render/?target=smartmeter.strom.*&from=-10s&format=csv```\
The above call renders three metrics: Current in all three Phases of my Smartmeter and outputs it in a CSV, which looks like this (without the header):
| Metric | Date Time | Value |
|---------------------|---------------------|-------|
| smartmeter.strom.L1 | 2022-03-05 10:07:30 | 24.54 |
| smartmeter.strom.L1 | 2022-03-05 10:07:35 | 24.53 |
| smartmeter.strom.L2 | 2022-03-05 10:07:30 | 1.84  |
| smartmeter.strom.L2 | 2022-03-05 10:07:35 | 1.8   |
| smartmeter.strom.L3 | 2022-03-05 10:07:30 | 0.93  |
| smartmeter.strom.L3 | 2022-03-05 10:07:35 | 0.93  |

## Solution
In this program i requesting the data in the format **raw** and output the formatted data in a usable CSV, which will look like this:
| time_smartmeter.strom.L1 | smartmeter.strom.L1 | time_smartmeter.strom.L2 | smartmeter.strom.L2 | time_smartmeter.strom.L3 | smartmeter.strom.L3 |
|--------------------------|---------------------|--------------------------|---------------------|--------------------------|---------------------|
| 05.03.2022 11:17:00      | 18.3                | 05.03.2022 11:17:00      | 1.91                | 05.03.2022 11:17:00      | 1.03                |
| 05.03.2022 11:17:05      | 1                   | 05.03.2022 11:17:05      | 2.06                | 05.03.2022 11:17:05      | 1.03                |

Notice that each metric has its own time column. This is because some metrics may have different retention times and thus would not match.

## Build
- cygwin x64
- cygwin CURL and REGEX is required

Build with
```
gcc main.c -lcurl
```

## Disclaimer
This program uses metrics/settings/ip addresses which will probably only fit for me. **You have to modify the code for your own purposes.**
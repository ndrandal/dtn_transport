# Parameter Reference

This document describes every JSON key emitted by the `ingest_server` service for both Level 1 (L1) and Level 2 (L2) feeds. Middleware clients can use these keys to extract the data they need.

---

## Common Metadata Fields

- **`feed`** (string)  
  Indicates which feed the message originated from: `"L1"` or `"L2"`.

- **`messageType`** (string)  
  The original message ID character or code.

---

## Message Type Codes

### L1 Message Type Codes
Refer to **L1FeedMessages.pdf** for the complete list of numeric codes and their meanings. Common codes include:

- `0`: Time Stamp Message
- `1`: System Status Message
- `2`: Market Maker Statistics (if enabled)
- `3`: Last Trade Update
- `4`: Quote Update
- ... (see documentation for full list)

### L2 Message Type Codes

| Code | Name                              | Description                                                         |
|------|-----------------------------------|---------------------------------------------------------------------|
| `0`  | Price Level Order                 | Detailed order information at a specific price level (Order Lookup) |
| `3`  | Order Add                         | Notification of a new order added to the book                       |
| `4`  | Order Update                      | Update to an existing order’s details                               |
| `5`  | Order Delete                      | Removal of an order from the book                                   |
| `6`  | Order Summary                     | Initial snapshot of all orders for a symbol                         |
| `7`  | Price Level Summary               | Initial snapshot of aggregated price level data                     |
| `8`  | Price Level Update                | Update to aggregated price level information                        |
| `9`  | Price Level Delete                | Removal of a price level from the aggregated depth                  |
| `n`  | Symbol Not Found                  | Indicates the requested symbol is invalid or not present            |
| `q`  | No Depth Available                | Watch established but no depth currently available                  |
| `M`  | Market Maker Description          | Response to an MMID description request, contains text description |
| `S`  | System / Clear Depth Message      | Various system notifications (e.g., clear all depth for a side)     |

---

## Level 1 (L1) Fields

| Key                             | Description                                      | Type    |
|---------------------------------|--------------------------------------------------|---------|
| `Message ID`                    | L1 message type identifier                       | string  |
| `Symbol`                        | Security ticker symbol                           | string  |
| `Most Recent Trade`             | Last trade price                                 | number  |
| `Most Recent Trade Size`        | Size of the last trade                           | integer |
| `Most Recent Trade Time`        | Time of last trade (HH:mm:ss.ffffff)             | string  |
| `Most Recent Trade Market Center` | Market center code for the last trade           | string  |
| `Total Volume`                  | Cumulative trading volume                        | integer |
| `Bid`                           | Current bid price                                | number  |
| `Bid Size`                      | Size available at the bid                        | integer |
| `Ask`                           | Current ask price                                | number  |
| `Ask Size`                      | Size available at the ask                        | integer |
| `Open`                          | Opening price                                    | number  |
| `High`                          | Highest price of session                         | number  |
| `Low`                           | Lowest price of session                          | number  |
| `Close`                         | Closing price                                    | number  |
| `Message Contents`              | Additional free-text message content             | string  |
| `Most Recent Trade Conditions`  | Trade condition codes for the last trade         | string  |

---

## Level 2 (L2) Depth Fields

| Key           | Description                                                                 | Type    |
|---------------|-----------------------------------------------------------------------------|---------|
| `Message Type`| L2 message type identifier (`0`,`3`,`4`,`5`,`6`,`7`,`8`,`9`)                | string  |
| `SYMBOL`      | Security ticker symbol                                                      | string  |
| `Order ID`    | Unique order identifier (for Market By Order)                                | integer |
| `MMID`        | Market Maker ID (blank for MBO Futures)                                     | string  |
| `Side`        | `"A"`=Ask/Sell or `"B"`=Bid/Buy                                              | string  |
| `Price`       | Price of the order or depth level                                           | number  |
| `Order Size`  | Size of a single order (for MBO messages)                                   | integer |
| `Order Priority` | Priority index of the order within its price level                        | integer |
| `Level Size`  | Total aggregated size at this price level (for price-level messages)         | integer |
| `Order Count` | Number of individual orders at this price level                              | integer |
| `Precision`   | Number of decimal places for the price field                                 | integer |
| `timestamp`   | Combined ISO 8601 date-time of the message (from L2 `Date` + `Time` fields)  | string  |

---

**Usage Example**

A middleware client subscribing to both feeds will first parse the top-level `feed` and `messageType` fields, then access the relevant keys above. For instance:

```json
{
  "feed": "L2",
  "messageType": "3",
  "SYMBOL": "AAPL",
  "Order ID": 123456789,
  "Side": "B",
  "Price": 157.25,
  "Order Size": 100,
  "timestamp": "2025-06-22T15:34:12.123456Z"
}

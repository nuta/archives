sakura.io
==========

MakeStack supports [sakura.io](https://sakura.io) as a network adapter. sakura.io enables you to deploy
new app version and, change configration, collect device log remotely over LTE without modifying
existing MakeStack app code.

Restrictions
------------

- The maximum length of a SMMS payload is 128 bytes; that is, buffered log over about 100 characters
  will be discarded.

How to use
----------

1. Add Outgoing Webhook in [sakura.io console](https://secure.sakura.ad.jp/iot/console). Set `https://YOUR-MAKESTACK-SERVER/api/v1/sakuraio/webhook` as the payload URL.
2. Add Incoming Webhook in [sakura.io console](https://secure.sakura.ad.jp/iot/console). Copy
   its `Token` and create sakura.io integration on MakeStack Server the app settings.
3. Add a file distribution setting in [sakura.io console](https://secure.sakura.ad.jp/iot/console).
  Set `https://YOUR-MAKESTACK-SERVER/api/v1/sakuraio/file` as file number 1 URL.
4. Copy the sakura.io communication module ID showed in [sakura.io console](https://secure.sakura.ad.jp/iot/console) to MakeStack Server the device settings.

# https://seiya.me

- [DigitalOcean](https://cloud.digitalocean.com/projects/4103f9fd-06a0-42dd-85e2-f0df3b8d8d1c/resources?i=22cd1f)
- [Google Analytics](https://analytics.google.com/analytics/web/#/p297393201/realtime/overview)
- [Grafana](https://seiya.grafana.net/?orgId=1)

```
cd infra && terraform init && cd ..
brew install terraform doctl yq jpeg optipng
pip3 install python-digitalocean requests
doctl auth init
```

```
make tf-apply
./infra/reboot-all.py
```

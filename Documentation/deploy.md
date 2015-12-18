# Deploy with Ubuntu 14.04, Nginx, and Unicorn

1. [Follow insturctions described here](https://www.digitalocean.com/community/tutorials/how-to-deploy-a-rails-app-with-unicorn-and-nginx-on-ubuntu-14-04)
2. write nginx config
3. add account in your calendar app

### Nginx config
```
upstream app {
    server unix:/home/user/busybook/unicorn/unicorn.sock fail_timeout=0;
}

server {
    listen 80;
    server_name localhost;

    root /home/user/busybook/public;

    try_files $uri/index.html $uri @app;

    location /busybook {
        rewrite /busybook/(.*) /$1 break;
        proxy_pass http://app;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header Host $http_host;
        proxy_redirect off;
    }
}
```

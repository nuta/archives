Rails.application.routes.draw do
  scope :api do
    if Rails.env.development?
      require 'sidekiq/web'
      mount Sidekiq::Web => '/sidekiq'
    end

    scope :v1 do
      mount_devise_token_auth_for 'User', at: 'auth'

      resources :apps, param: :name do
        resources :deployments,  param: :version, only: %i(index show create)
        resources :app_stores,   param: :key, path: 'stores', as: :stores, constraints: { key: /.+/ }
        resources :integrations, param: :service
        resources :source_files, param: :path, path: 'files', format: false, constraints: { path: /.+/ }
        get "log", to: "apps#log"
      end

      resources :devices, param: :name do
        resources :device_stores, param: :key, path: 'stores', as: :store, constraints: { key: /.+/ }
        get "log", to: "devices#log"
      end

      post "heartbeat", to: "heartbeat#heartbeat"
      get "os_image",   to: "heartbeat#os_image"
      get "app_image",  to: "heartbeat#app_image"

      get "webhooks/*token", to: "incoming_webhooks#invoke", constraints: { token: /.+/ }
  end
  end
end

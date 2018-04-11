Rails.application.routes.draw do
  scope :api do
    if Rails.env.development?
      require 'sidekiq/web'
      mount Sidekiq::Web => '/sidekiq'
    end

    scope :v1 do
      get 'health', to: 'health#health'

      mount_devise_token_auth_for 'User', at: 'auth',
        controllers: { registrations: 'auth/registrations' }

      resources :apps, param: :name do
        resources :deployments,  controller: 'apps/deployments',
          param: :version, only: %i(index show create)

        resources :app_configs, controller: 'apps/configs',
          param: :key, path: 'configs', as: :configs, constraints: { key: /.+/ },
          only: %i(index show update destroy)

        resources :integrations, controller: 'apps/integrations', param: :name

        resources :log, controller: 'apps/log', only: [:index]
      end

      resources :devices, param: :name do
        resources :device_configs, controller: 'devices/configs',
          param: :key, path: 'configs', as: :config, constraints: { key: /.+/ },
          only: %i(index show update destroy)

        resources :log, controller: 'devices/log', only: [:index]
        resources :commands, controller: 'devices/commands', only: [:index, :create]
      end

      post "smms", to: "smms#http"
      scope :images do
        get "/os/:device_id/:version/:device_type", to: "images#os_image", constraints: { device_id: /.+/ }
        get "/app/:device_id/:version",  to: "images#app_image", constraints: { device_id: /.+/ }
      end

      post "webhooks/*token", to: "incoming_webhooks#invoke", constraints: { token: /.+/ }

      scope :sakuraio do
        post 'webhook', to: 'sakuraio#webhook'
        get 'file', to: 'sakuraio#file'
      end

      scope :os do
        get 'releases', to: "os#index"
      end
    end
  end
end

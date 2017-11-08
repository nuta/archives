Rails.application.routes.draw do
  scope :api do
    if Rails.env.development?
      require 'sidekiq/web'
      mount Sidekiq::Web => '/sidekiq'
    end

    scope :v1 do
      mount_devise_token_auth_for 'User', at: 'auth',
        controllers: { registrations: 'auth/registrations' }

      resources :apps, param: :name do
        resources :deployments,  controller: 'apps/deployments',
          param: :version, only: %i(index show create)

        resources :app_stores, controller: 'apps/stores',
          param: :key, path: 'stores', as: :stores, constraints: { key: /.+/ }

        resources :integrations, controller: 'apps/integrations', param: :name

        resources :source_files, controller: 'apps/source_files',
          param: :path, path: 'files', format: false,
         constraints: { path: /.+/ }, only: [:index, :show, :update, :destroy]

        resources :log, controller: 'apps/log', only: [:index]
      end

      resources :devices, param: :name do
        resources :device_stores, controller: 'devices/stores',
          param: :key, path: 'stores', as: :store, constraints: { key: /.+/ }

        resources :log, controller: 'devices/log', only: [:index]
      end

      post "smms", to: "smms#http"
      scope :images do
        get "/os/:device_id/:version/:os/:device_type", to: "images#os_image", constraints: { device_id: /.+/ }
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

      scope :plugins do
        get '/:org/:repo/:name', to: "plugins#download"
      end
    end
  end
end

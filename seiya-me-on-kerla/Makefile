.PHONY:
help:
	echo "See README.md"

ifeq ($(V),)
.SILENT:
endif

MAKEFLAGS += --no-builtin-rules --no-builtin-variables
.SUFFIXES:

PROGRESS := printf "  \\033[1;96m%10s\\033[0m  \\033[1;m%s\\033[0m\\n"

ifeq ($(shell uname),Darwin)
DO_TOKEN ?= $(shell yq e .access-token ~/Library/Application\ Support/doctl/config.yaml)
endif

KERLA_TAG ?= main

TERRAFORM_FLAGS += -var "do_token=$(DO_TOKEN)"
TERRAFORM_FLAGS += -var "grafana_cloud_username=$(GRAFANA_CLOUD_USERNAME)"
TERRAFORM_FLAGS += -var "grafana_cloud_password=$(GRAFANA_CLOUD_PASSWORD)"

os:
	$(MAKE) seiya-me
	$(MAKE) kerla
	$(MAKE) -C build/kerla IMAGE=seiya-me RELEASE=1

kerla:
	$(PROGRESS) BUILD $@
	mkdir build
	git clone https://github.com/nuta/kerla build/kerla
	cd build/kerla && git checkout $(KERLA_TAG)

seiya-me:
	$(PROGRESS) BUILD $@
	docker buildx build -t seiya-me .

.PHONY: tf-plan
tf-plan:
	$(PROGRESS) TERRAFORM PLAN
	cd infra && terraform plan $(TERRAFORM_FLAGS)

.PHONY: tf-apply
tf-apply:
	if [ "$$DO_TOKEN" = "" ]; then \
		echo DO_TOKEN is not set; \
		exit 1; \
	fi
	if [ "$$GRAFANA_CLOUD_USERNAME" = "" ]; then \
		echo GRAFANA_CLOUD_USERNAME is not set; \
		exit 1; \
	fi
	if [ "$$GRAFANA_CLOUD_PASSWORD" = "" ]; then \
		echo GRAFANA_CLOUD_PASSWORD is not set; \
		exit 1; \
	fi
	$(PROGRESS) TERRAFORM APPLY
	cd infra && terraform apply $(TERRAFORM_FLAGS)

.PHONY: optimize-images
optimize-images:
	$(PROGRESS) "OPTIMIZE"
	echo blog/media/*.jpg | xargs -I '{}' -n 1 -P $(shell nproc) bash -c "jpegtran -copy none -optimize -progressive -outfile {}.tmp {}; mv {}.tmp {}"
	echo blog/media/*.png | xargs -I '{}' -n 1 -P $(shell nproc) bash -c "optipng -out {}.tmp {}; mv {}.tmp {}"

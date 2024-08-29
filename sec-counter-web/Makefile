SEC_COUNTER_BIN := bin/sec-counter-web

GO ?= go

VERSION := $(shell cat VERSION)
USE_VENDOR =
LOCAL_LDFLAGS = -buildmode=pie -ldflags "-X=main.Version=$(VERSION)"

.PHONY: all api build vendor
all: dep build

dep: ## Get the dependencies
	@$(GO) get -v -d ./...

update: ## Get and update the dependencies
	@$(GO) get -v -d -u ./...

tidy: ## Clean up dependencies
	@$(GO) mod tidy

vendor: dep ## Create vendor directory
	@$(GO) mod vendor

build: ## Build the binary files
	$(GO) build -v -o $(SEC_COUNTER_BIN) $(USE_VENDOR) $(LOCAL_LDFLAGS) ./cmd

clean: ## Remove previous builds
	@rm -f $(SEC_COUNTER_BIN)

help: ## Display this help screen
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'


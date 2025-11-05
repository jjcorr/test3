#!/usr/bin/env bash
set -euo pipefail

NAMESPACE=${1:-default}

echo "Applying Kubernetes manifests to namespace: $NAMESPACE"
kubectl apply -f kube/deployment.yaml -n "$NAMESPACE"

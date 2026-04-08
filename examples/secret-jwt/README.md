# secret-jwt

Issue HS256 JWTs signed with a secret stored in the domain's secret store.
Returns the token as `application/jwt` with `sub` and `iss` claims.

**Required configuration**: add `jwt-key` to `allowed_secrets`. Store the
HMAC key value via the Flaron dashboard before deploying.

create table if not exists public.dryer_state (
    device_id text primary key,
    status jsonb not null,
    last_seen timestamptz not null default now(),
    pending_command jsonb,
    last_command_ack jsonb
);

alter table public.dryer_state enable row level security;
revoke all on table public.dryer_state from anon, authenticated;
grant all on table public.dryer_state to service_role;

create or replace function public.device_exchange(
    p_device_id text,
    p_status jsonb,
    p_ack jsonb default null
)
returns jsonb
language plpgsql
security definer
set search_path = public, pg_temp
as $$
declare
    next_command jsonb;
begin
    insert into public.dryer_state as current_state (
        device_id,
        status,
        last_seen,
        last_command_ack
    )
    values (
        p_device_id,
        p_status,
        now(),
        p_ack
    )
    on conflict (device_id) do update
    set status = excluded.status,
        last_seen = excluded.last_seen,
        last_command_ack = coalesce(
            excluded.last_command_ack,
        current_state.last_command_ack
        );

    select pending_command
    into next_command
    from public.dryer_state
    where device_id = p_device_id
    for update;

    if next_command is null then
        return null;
    end if;

    if nullif(next_command ->> 'expiresAt', '') is null
       or (next_command ->> 'expiresAt')::timestamptz <= now() then
        update public.dryer_state
        set pending_command = null
        where device_id = p_device_id;
        return null;
    end if;

    update public.dryer_state
    set pending_command = null
    where device_id = p_device_id;

    return next_command;
end;
$$;

create or replace function public.queue_dryer_command(
    p_device_id text,
    p_command jsonb
)
returns boolean
language plpgsql
security definer
set search_path = public, pg_temp
as $$
begin
    update public.dryer_state
    set pending_command = p_command
    where device_id = p_device_id
      and last_seen > now() - interval '25 seconds'
      and (
          pending_command is null
          or nullif(pending_command ->> 'expiresAt', '') is null
          or (pending_command ->> 'expiresAt')::timestamptz <= now()
      );

    return found;
end;
$$;

revoke all on function public.device_exchange(text, jsonb, jsonb)
    from public, anon, authenticated;
revoke all on function public.queue_dryer_command(text, jsonb)
    from public, anon, authenticated;
grant execute on function public.device_exchange(text, jsonb, jsonb)
    to service_role;
grant execute on function public.queue_dryer_command(text, jsonb)
    to service_role;
